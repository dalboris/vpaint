// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "Timeline.h"
#include "Scene.h"
#include "SceneObject.h"
#include <QKeyEvent>

#include "VectorAnimationComplex/VAC.h"
#include "VectorAnimationComplex/InbetweenFace.h"
#include "Background/Background.h"

#include <QtDebug>

#include "XmlStreamReader.h"
#include "XmlStreamWriter.h"

#include "OpenGL.h"
#include "Global.h"

Scene::Scene() :
    left_(0),
    top_(0),
    width_(1280),
    height_(720),
    background_(new Background(this))
{
    VectorAnimationComplex::VAC * vac = new VectorAnimationComplex::VAC();
    connect(vac,SIGNAL(selectionChanged()),this,SIGNAL(selectionChanged()));
    connect(background_, SIGNAL(changed()), this, SIGNAL(changed()));
    connect(background_, SIGNAL(checkpoint()), this, SIGNAL(checkpoint()));
    addSceneObject(vac);
    indexHovered_ = -1;
}

double Scene::left() const
{
    return left_;
}

double Scene::top() const
{
    return top_;
}

double Scene::width() const
{
    return width_;
}

double Scene::height() const
{
    return height_;
}

void Scene::setLeft(double x)
{
    left_ = x;
    emitChanged();
}

void Scene::setTop(double y)
{
    top_ = y;
    emitChanged();
}

void Scene::setWidth(double w)
{
    width_ = w;
    emitChanged();
}

void Scene::setHeight(double h)
{
    height_ = h;
    emitChanged();
}

Background * Scene::background() const
{
    return background_;
}

void Scene::setCanvasDefaultValues()
{
    left_ = 0;
    top_ = 0;
    width_ = 1280;
    height_ = 720;

    // Don't emit changed on purpose
}

void Scene::copyFrom(Scene * other)
{
    // XXX
    // In this method, here's is what's wrong:
    //  - canvas is not copied
    //  - VAC is should copied in a cleaner way (take Background as a model)

    // Block signals
    blockSignals(true);

    // Reset to default
    clear(true);

    // Copy VAC
    foreach(SceneObject *sceneObject, other->sceneObjects_)
        addSceneObject(sceneObject->clone(), true);

    // Reset hovered
    indexHovered_ = -1;

    // Copy background
    background_->setData(other->background_);

    // Unblock signals
    blockSignals(false);

    // Emit signals
    emit needUpdatePicking();
    emitChanged();

    // Create new connections
    VectorAnimationComplex::VAC * vac = getVAC_();
    if(vac)
    {
        connect(vac,SIGNAL(selectionChanged()),this,SIGNAL(selectionChanged()));
        emit selectionChanged();
    }
}

void Scene::clear(bool silent)
{
    VectorAnimationComplex::VAC * vac = getVAC_();
    if(vac)
        disconnect(vac,SIGNAL(selectionChanged()),this,SIGNAL(selectionChanged()));

    foreach(SceneObject *sceneObject, sceneObjects_)
        delete sceneObject;
    sceneObjects_.clear();

    // XXX Shouldn't this clear left/top/width/height too?

    // Reset background data
    // As a side effect, this clears the cache if there were any
    background_->resetData();

    if(!silent)
    {
        emitChanged();
        emit needUpdatePicking();
        emit selectionChanged();
    }
}

Scene::~Scene()
{
    clear(true);
}

// ----------------------- Save and Load -------------------------
#include "SaveAndLoad.h"

void Scene::save(QTextStream & out)
{
    out << Save::newField("SceneObjects");
    out << "\n" << Save::indent() << "[";
    Save::incrIndent();
    foreach(SceneObject *sceneObject, sceneObjects_)
    {
        out << Save::openCurlyBrackets();
        sceneObject->save(out);
        out << Save::closeCurlyBrackets();
    }
    Save::decrIndent();
    out << "\n" << Save::indent() << "]";
}


void Scene::exportSVG(Time t, QTextStream & out)
{
    foreach(SceneObject *sceneObject, sceneObjects_)
    {
        sceneObject->exportSVG(t, out);
    }
}


void Scene::read(QTextStream & in)
{
    clear(true);
    
    QString field;
    QString bracket;
    field = Read::field(in);
    Read::skipBracket(in); // [
    while(Read::string(in) == "{")
    {
        addSceneObject(SceneObject::read(in), true);
        Read::skipBracket(in); // }
    }
    // if here, last read string == ]


    VectorAnimationComplex::VAC * vac = getVAC_();
    if(vac)
    {
        connect(vac,SIGNAL(selectionChanged()),this,SIGNAL(selectionChanged()));
    }

    emit changed();
    emit needUpdatePicking();
    emit selectionChanged();
}

void Scene::write(XmlStreamWriter & xml)
{
    // Background
    xml.writeStartElement("background");
    background()->write(xml);
    xml.writeEndElement();

    // Vector animation complex
    xml.writeStartElement("objects");
    vectorAnimationComplex()->write(xml);
    xml.writeEndElement();
}

void Scene::read(XmlStreamReader & xml)
{
    blockSignals(true);

    clear(true);

    while (xml.readNextStartElement())
    {
        if (xml.name() == "background")
        {
            background()->read(xml);
        }
        else if (xml.name() == "objects")
        {
            VectorAnimationComplex::VAC * vac = new VectorAnimationComplex::VAC();
            vac->read(xml);
            addSceneObject(vac, true);
            connect(vac,SIGNAL(selectionChanged()),this,SIGNAL(selectionChanged()));
        }
        else
        {
            xml.skipCurrentElement();
        }
    }

    blockSignals(false);

    emit needUpdatePicking();
    emit changed();
    emit selectionChanged();
}

void Scene::readCanvas(XmlStreamReader & xml)
{

    setCanvasDefaultValues();

    // Canvas
    if(xml.attributes().hasAttribute("left"))
        setLeft(xml.attributes().value("left").toDouble());
    if(xml.attributes().hasAttribute("top"))
        setTop(xml.attributes().value("top").toDouble());
    if(xml.attributes().hasAttribute("width"))
        setWidth(xml.attributes().value("width").toDouble());
    if(xml.attributes().hasAttribute("height"))
        setHeight(xml.attributes().value("height").toDouble());

    xml.skipCurrentElement();
}

void Scene::writeCanvas(XmlStreamWriter & xml)
{
    xml.writeAttribute("left", QString().setNum(left()));
    xml.writeAttribute("top", QString().setNum(top()));
    xml.writeAttribute("width", QString().setNum(width()));
    xml.writeAttribute("height", QString().setNum(height()));
}

void Scene::relativeRemap(const QDir & oldDir, const QDir & newDir)
{
    background()->relativeRemap(oldDir, newDir);
}

// ----------------------- Drawing the scene -------------------------

// XXX Refactor this: move it to View. Even better, have a Canvas and
// CanvasRenderer class
void Scene::drawCanvas(ViewSettings & /*viewSettings*/)
{
    double x = left();
    double y = top();
    double w = width();
    double h = height();

    if(global()->showCanvas())
    {
        // Out-of-canvas background color
        glClearColor(0.8, 0.8, 0.8, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        // Canvas border
        glColor4d(0.0, 0.0, 0.0, 1.0);
        glLineWidth(3.0);
        glBegin(GL_LINE_LOOP);
        {
            glVertex2d(x,y);
            glVertex2d(x+w,y);
            glVertex2d(x+w,y+h);
            glVertex2d(x,y+h);
        }
        glEnd();

        // Canvas color
        glColor4d(1.0, 1.0, 1.0, 1.0);
        glBegin(GL_QUADS);
        {
            glVertex2d(x,y);
            glVertex2d(x+w,y);
            glVertex2d(x+w,y+h);
            glVertex2d(x,y+h);
        }
        glEnd();
    }
    else
    {
        // Canvas color
        glColor4d(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
    }
}

void Scene::draw(Time time, ViewSettings & viewSettings)
{
    // Draw VAC
    // XXX this was over-engineered. Should revert to something simpler:
    //  vac_->draw(time, viewSettings);
    // and later:
    //   foreach(Layer * layer, layers_)
    //     layer->draw(time, viewSettings);
    foreach(SceneObject *sceneObject, sceneObjects_)
    {
        sceneObject->draw(time, viewSettings);
    }
}

void Scene::drawPick(Time time, ViewSettings & viewSettings)
{
    for(int i=0; i<sceneObjects_.size(); i++)
    {
        Picking::setIndex(i);
          sceneObjects_[i]->drawPick(time, viewSettings);
    }
}


// ---------------- Highlighting and Selecting -----------------------
    
// No need to emit changed() or needUpdatePicking() here, since highlighting and selecting
// is trigerred by View or View3D, and hence they can decide themselves what do they
// need to update

void Scene::setHoveredObject(Time time, int index, int id)
{
    setNoHoveredObject();
    indexHovered_ = index;
    sceneObjects_[index]->setHoveredObject(time, id);
}

void Scene::setNoHoveredObject()
{
    if(indexHovered_ != -1)
    {
        sceneObjects_[indexHovered_]->setNoHoveredObject();
        indexHovered_ = -1;
    }
}
void Scene::select(Time time, int index, int id)
{
    sceneObjects_[index]->select(time, id);
}
void Scene::deselect(Time time, int index, int id)
{
    sceneObjects_[index]->deselect(time, id);
}
void Scene::toggle(Time time, int index, int id)
{
    sceneObjects_[index]->toggle(time, id);
}
void Scene::deselectAll(Time time)
{
    foreach(SceneObject * so, sceneObjects_)
        so->deselectAll(time);
}
void Scene::deselectAll()
{
    foreach(SceneObject * so, sceneObjects_)
        so->deselectAll();
}
void Scene::invertSelection()
{
    foreach(SceneObject * so, sceneObjects_)
        so->invertSelection();
}


// ---------------- VAC specific Selection -----------------------


void Scene::selectAll()
{
    if(!sceneObjects_.isEmpty())
    {
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
            vac->selectAll();
    }
}

void Scene::selectConnected()
{
    if(!sceneObjects_.isEmpty())
    {
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
            vac->selectConnected();
    }
}

void Scene::selectClosure()
{
    if(!sceneObjects_.isEmpty())
    {
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
            vac->selectClosure();
    }
}

void Scene::selectVertices()
{
    if(!sceneObjects_.isEmpty())
    {
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
            vac->selectVertices();
    }
}

void Scene::selectEdges()
{
    if(!sceneObjects_.isEmpty())
    {
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
            vac->selectEdges();
    }
}

void Scene::selectFaces()
{
    if(!sceneObjects_.isEmpty())
    {
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
            vac->selectFaces();
    }
}

void Scene::deselectVertices()
{
    if(!sceneObjects_.isEmpty())
    {
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
            vac->deselectVertices();
    }
}

void Scene::deselectEdges()
{
    if(!sceneObjects_.isEmpty())
    {
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
            vac->deselectEdges();
    }
}

void Scene::deselectFaces()
{
    if(!sceneObjects_.isEmpty())
    {
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
            vac->deselectFaces();
    }
}


// ----------------------- User Interactions -------------------------

void Scene::keyPressEvent(QKeyEvent *event)
{
    event->ignore();
}

void Scene::keyReleaseEvent(QKeyEvent *event)
{
    event->ignore();
}

VectorAnimationComplex::VAC * Scene::vectorAnimationComplex()
{
    return getVAC_();
}

void Scene::addSceneObject(SceneObject * sceneObject, bool silent)
{
    sceneObjects_ << sceneObject;
    connect(sceneObject, SIGNAL(changed()),
          this, SIGNAL(changed()));
    connect(sceneObject, SIGNAL(checkpoint()),
          this, SIGNAL(checkpoint()));
    connect(sceneObject, SIGNAL(needUpdatePicking()),
          this, SIGNAL(needUpdatePicking()));
    if(!silent)
    {
        emitChanged();
        emit needUpdatePicking();
    }
}

#include <QToolBar>

void Scene::populateToolBar(QToolBar * toolBar)
{
    // Actions of the whole scene
    // put undo redo here

    // Actions of specific scene objects
    VectorAnimationComplex::VAC::populateToolBar(toolBar, this);
}

void Scene::deleteSelectedCells()
{
    if(!sceneObjects_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);
        
        if(vac)
        {
            vac->deleteSelectedCells();
        }
    }
    
}

void Scene::test()
{
    if(!sceneObjects_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
        {
            vac->test();
        }
    }

}

void Scene::smartDelete()
{
    if(!sceneObjects_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
        {
            vac->smartDelete();
        }
    }

}

VectorAnimationComplex::VAC * Scene::getVAC_()
{
    using VectorAnimationComplex::VAC;

    if(sceneObjects_.isEmpty())
    {
        return 0;
    }
    else
    {
        SceneObject * sceneObject = sceneObjects_[0];
        VAC * vac = dynamic_cast<VAC *>(sceneObject);
        return vac;
    }
}

VectorAnimationComplex::InbetweenFace * Scene::createInbetweenFace()
{
    VectorAnimationComplex::VAC * vac = getVAC_();

    if(vac)
    {
        return vac->newInbetweenFace(QList<VectorAnimationComplex::AnimatedCycle>(),
                                       QSet<VectorAnimationComplex::KeyFace*>(),
                                       QSet<VectorAnimationComplex::KeyFace*>());
    }
    else
    {
        return 0;
    }
}

void Scene::cut(VectorAnimationComplex::VAC* & clipboard)
{
    VectorAnimationComplex::VAC * vac = getVAC_();
    if(vac)
    {
        vac->cut(clipboard);
    }
}

void Scene::copy(VectorAnimationComplex::VAC* & clipboard)
{
    VectorAnimationComplex::VAC * vac = getVAC_();
    if(vac)
    {
        vac->copy(clipboard);
    }
}

void Scene::paste(VectorAnimationComplex::VAC* & clipboard)
{
    VectorAnimationComplex::VAC * vac = getVAC_();
    if(vac)
    {
        vac->paste(clipboard);
    }
}

void Scene::motionPaste(VectorAnimationComplex::VAC* & clipboard)
{
    VectorAnimationComplex::VAC * vac = getVAC_();
    if(vac)
    {
        vac->motionPaste(clipboard);
    }
}


void Scene::createFace()
{

    if(!sceneObjects_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
        {
            vac->createFace();
            // currently, signals for updating view and picking are called
            // inside the previous call. Not sure if it's a good idea.
        }
    }

}

void Scene::addCyclesToFace()
{
    if(!sceneObjects_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
        {
            vac->addCyclesToFace();
            // currently, signals for updating view and picking are called
            // inside the previous call. Not sure if it's a good idea.
        }
    }

}

void Scene::removeCyclesFromFace()
{
    if(!sceneObjects_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
        {
            vac->removeCyclesFromFace();
            // currently, signals for updating view and picking are called
            // inside the previous call. Not sure if it's a good idea.
        }
    }

}

void Scene::changeColor()
{
    if(!sceneObjects_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
        {
            vac->changeColor();
            // currently, signals for updating view and picking are called
            // inside the previous call. Not sure if it's a good idea.
        }
    }

}
void Scene::raise()
{
    if(!sceneObjects_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
        {
            vac->raise();
            // currently, signals for updating view and picking are called
            // inside the previous call. Not sure if it's a good idea.
        }
    }

}
void Scene::lower()
{
    if(!sceneObjects_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
        {
            vac->lower();
            // currently, signals for updating view and picking are called
            // inside the previous call. Not sure if it's a good idea.
        }
    }

}

void Scene::raiseToTop()
{
    if(!sceneObjects_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
        {
            vac->raiseToTop();
            // currently, signals for updating view and picking are called
            // inside the previous call. Not sure if it's a good idea.
        }
    }

}
void Scene::lowerToBottom()
{
    if(!sceneObjects_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
        {
            vac->lowerToBottom();
            // currently, signals for updating view and picking are called
            // inside the previous call. Not sure if it's a good idea.
        }
    }

}
void Scene::altRaise()
{
    if(!sceneObjects_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
        {
            vac->altRaise();
            // currently, signals for updating view and picking are called
            // inside the previous call. Not sure if it's a good idea.
        }
    }

}
void Scene::altLower()
{
    if(!sceneObjects_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
        {
            vac->altLower();
            // currently, signals for updating view and picking are called
            // inside the previous call. Not sure if it's a good idea.
        }
    }

}

void Scene::altRaiseToTop()
{
    if(!sceneObjects_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
        {
            vac->altRaiseToTop();
            // currently, signals for updating view and picking are called
            // inside the previous call. Not sure if it's a good idea.
        }
    }

}
void Scene::altLowerToBottom()
{
    if(!sceneObjects_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
        {
            vac->altLowerToBottom();
            // currently, signals for updating view and picking are called
            // inside the previous call. Not sure if it's a good idea.
        }
    }

}

void Scene::changeEdgeWidth()
{
    if(!sceneObjects_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
        {
            vac->changeEdgeWidth();
            // currently, signals for updating view and picking are called
            // inside the previous call. Not sure if it's a good idea.
        }
    }

}

void Scene::glue()
{

    if(!sceneObjects_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
        {
            vac->glue();
            // currently, signals for updating view and picking are called
            // inside the previous call. Not sure if it's a good idea.
        }
    }

}

void Scene::unglue()
{

    if(!sceneObjects_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
        {
            vac->unglue();
            // currently, signals for updating view and picking are called
            // inside the previous call. Not sure if it's a good idea.
        }
    }

}

void Scene::uncut()
{

    if(!sceneObjects_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
        {
            vac->uncut();
            // currently, signals for updating view and picking are called
            // inside the previous call. Not sure if it's a good idea.
        }
    }

}

void Scene::inbetweenSelection()
{
    if(!sceneObjects_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
        {
            vac->inbetweenSelection();
            // currently, signals for updating view and picking are called
            // inside the previous call. Not sure if it's a good idea.
        }
    }
}

void Scene::keyframeSelection()
{
    if(!sceneObjects_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
        {
            vac->keyframeSelection();
            // currently, signals for updating view and picking are called
            // inside the previous call. Not sure if it's a good idea.
        }
    }
}


void Scene::resetCellsToConsiderForCutting()
{

    if(!sceneObjects_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
        {
            vac->resetCellsToConsiderForCutting();
            // currently, signals for updating view and picking are called
            // inside the previous call. Not sure if it's a good idea.
        }
    }

}

void Scene::updateCellsToConsiderForCutting()
{

    if(!sceneObjects_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (sceneObjects_[0]);

        if(vac)
        {
            vac->updateCellsToConsiderForCutting();
            // currently, signals for updating view and picking are called
            // inside the previous call. Not sure if it's a good idea.
        }
    }

}

    

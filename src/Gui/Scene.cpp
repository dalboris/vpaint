// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

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

#include "SaveAndLoad.h"

#include <QToolBar>

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
    addLayer(vac);
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

    // Copy layers
    foreach(SceneObject * layer, other->layers_)
        addLayer(layer->clone(), true);

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
    VectorAnimationComplex::VAC * vac = activeLayer();
    if(vac)
    {
        connect(vac,SIGNAL(selectionChanged()),this,SIGNAL(selectionChanged()));
        emit selectionChanged();
    }
}

void Scene::clear(bool silent)
{
    VectorAnimationComplex::VAC * vac = activeLayer();
    if(vac)
        disconnect(vac,SIGNAL(selectionChanged()),this,SIGNAL(selectionChanged()));

    foreach(SceneObject * layer, layers_)
        delete layer;
    layers_.clear();

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

void Scene::save(QTextStream & out)
{
    out << Save::newField("SceneObjects");
    out << "\n" << Save::indent() << "[";
    Save::incrIndent();
    foreach(SceneObject * layer, layers_)
    {
        out << Save::openCurlyBrackets();
        layer->save(out);
        out << Save::closeCurlyBrackets();
    }
    Save::decrIndent();
    out << "\n" << Save::indent() << "]";
}


void Scene::exportSVG(Time t, QTextStream & out)
{
    // Export background
    background_->exportSVG(t.frame(), out,
                           left(), top(), width(), height());

    // Export Layers
    foreach(SceneObject * layer, layers_)
    {
        layer->exportSVG(t, out);
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
        addLayer(SceneObject::read(in), true);
        Read::skipBracket(in); // }
    }
    // if here, last read string == ]


    VectorAnimationComplex::VAC * vac = activeLayer();
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
    activeLayer()->write(xml);
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
            addLayer(vac, true);
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
    if(xml.attributes().hasAttribute("position"))
    {
        QString stringPos = xml.attributes().value("position").toString();
        QStringList list = stringPos.split(" ");
        setLeft(list[0].toDouble());
        setTop(list[1].toDouble());
    }
    if(xml.attributes().hasAttribute("size"))
    {
        QString stringsize = xml.attributes().value("size").toString();
        QStringList list = stringsize.split(" ");
        setWidth(list[0].toDouble());
        setHeight(list[1].toDouble());
    }

    xml.skipCurrentElement();
}

void Scene::writeCanvas(XmlStreamWriter & xml)
{
    xml.writeAttribute("position", QString().setNum(left()) + " " + QString().setNum(top()));
    xml.writeAttribute("size", QString().setNum(width()) + " " + QString().setNum(height()));
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
        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
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
    // Draw layers
    foreach(SceneObject *layer, layers_)
    {
        layer->draw(time, viewSettings);
    }
}

void Scene::drawPick(Time time, ViewSettings & viewSettings)
{
    for(int i=0; i<layers_.size(); i++)
    {
        Picking::setIndex(i);
          layers_[i]->drawPick(time, viewSettings);
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
    layers_[index]->setHoveredObject(time, id);
}

void Scene::setNoHoveredObject()
{
    if(indexHovered_ != -1)
    {
        layers_[indexHovered_]->setNoHoveredObject();
        indexHovered_ = -1;
    }
}
void Scene::select(Time time, int index, int id)
{
    layers_[index]->select(time, id);
}
void Scene::deselect(Time time, int index, int id)
{
    layers_[index]->deselect(time, id);
}
void Scene::toggle(Time time, int index, int id)
{
    layers_[index]->toggle(time, id);
}
void Scene::deselectAll(Time time)
{
    foreach(SceneObject * so, layers_)
        so->deselectAll(time);
}
void Scene::deselectAll()
{
    foreach(SceneObject * so, layers_)
        so->deselectAll();
}
void Scene::invertSelection()
{
    foreach(SceneObject * so, layers_)
        so->invertSelection();
}


// ---------------- VAC specific Selection -----------------------


void Scene::selectAll()
{
    if(!layers_.isEmpty())
    {
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

        if(vac)
            vac->selectAll();
    }
}

void Scene::selectConnected()
{
    if(!layers_.isEmpty())
    {
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

        if(vac)
            vac->selectConnected();
    }
}

void Scene::selectClosure()
{
    if(!layers_.isEmpty())
    {
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

        if(vac)
            vac->selectClosure();
    }
}

void Scene::selectVertices()
{
    if(!layers_.isEmpty())
    {
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

        if(vac)
            vac->selectVertices();
    }
}

void Scene::selectEdges()
{
    if(!layers_.isEmpty())
    {
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

        if(vac)
            vac->selectEdges();
    }
}

void Scene::selectFaces()
{
    if(!layers_.isEmpty())
    {
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

        if(vac)
            vac->selectFaces();
    }
}

void Scene::deselectVertices()
{
    if(!layers_.isEmpty())
    {
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

        if(vac)
            vac->deselectVertices();
    }
}

void Scene::deselectEdges()
{
    if(!layers_.isEmpty())
    {
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

        if(vac)
            vac->deselectEdges();
    }
}

void Scene::deselectFaces()
{
    if(!layers_.isEmpty())
    {
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

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

void Scene::addLayer(SceneObject * layer, bool silent)
{
    layers_ << layer;
    connect(layer, SIGNAL(changed()),
          this, SIGNAL(changed()));
    connect(layer, SIGNAL(checkpoint()),
          this, SIGNAL(checkpoint()));
    connect(layer, SIGNAL(needUpdatePicking()),
          this, SIGNAL(needUpdatePicking()));
    if(!silent)
    {
        emitChanged();
        emit needUpdatePicking();
    }
}

void Scene::populateToolBar(QToolBar * toolBar)
{
    // Actions of the whole scene
    // put undo redo here

    // Actions of specific scene objects
    VectorAnimationComplex::VAC::populateToolBar(toolBar, this);
}

void Scene::deleteSelectedCells()
{
    if(!layers_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);
        
        if(vac)
        {
            vac->deleteSelectedCells();
        }
    }
    
}

void Scene::test()
{
    if(!layers_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

        if(vac)
        {
            vac->test();
        }
    }

}

void Scene::smartDelete()
{
    if(!layers_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

        if(vac)
        {
            vac->smartDelete();
        }
    }

}

VectorAnimationComplex::VAC * Scene::activeLayer()
{
    using VectorAnimationComplex::VAC;

    if(layers_.isEmpty())
    {
        return 0;
    }
    else
    {
        SceneObject * layer = layers_[0];
        VAC * vac = dynamic_cast<VAC *>(layer);
        return vac;
    }
}

VectorAnimationComplex::InbetweenFace * Scene::createInbetweenFace()
{
    VectorAnimationComplex::VAC * vac = activeLayer();

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
    VectorAnimationComplex::VAC * vac = activeLayer();
    if(vac)
    {
        vac->cut(clipboard);
    }
}

void Scene::copy(VectorAnimationComplex::VAC* & clipboard)
{
    VectorAnimationComplex::VAC * vac = activeLayer();
    if(vac)
    {
        vac->copy(clipboard);
    }
}

void Scene::paste(VectorAnimationComplex::VAC* & clipboard)
{
    VectorAnimationComplex::VAC * vac = activeLayer();
    if(vac)
    {
        vac->paste(clipboard);
    }
}

void Scene::motionPaste(VectorAnimationComplex::VAC* & clipboard)
{
    VectorAnimationComplex::VAC * vac = activeLayer();
    if(vac)
    {
        vac->motionPaste(clipboard);
    }
}


void Scene::createFace()
{

    if(!layers_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

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
    if(!layers_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

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
    if(!layers_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

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
    if(!layers_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

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
    if(!layers_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

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
    if(!layers_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

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
    if(!layers_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

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
    if(!layers_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

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
    if(!layers_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

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
    if(!layers_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

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
    if(!layers_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

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
    if(!layers_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

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
    if(!layers_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

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

    if(!layers_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

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

    if(!layers_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

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

    if(!layers_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

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
    if(!layers_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

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
    if(!layers_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

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

    if(!layers_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

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

    if(!layers_.isEmpty())
    {
        // todo:  get  the  selected  one  instead  of  the  first
        VectorAnimationComplex::VAC * vac =
            dynamic_cast<VectorAnimationComplex::VAC *>
            (layers_[0]);

        if(vac)
        {
            vac->updateCellsToConsiderForCutting();
            // currently, signals for updating view and picking are called
            // inside the previous call. Not sure if it's a good idea.
        }
    }

}

    

// Copyright (C) 2012-2019 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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

#include "Layer.h"
namespace VPaint {
Scene::Scene() :
    activeLayerIndex_(-1),
    left_(0),
    top_(0),
    width_(1280),
    height_(720)
{
    indexHovered_ = -1;
}

Scene * Scene::createDefaultScene()
{
    Scene * res = new Scene();
    Layer * layer = res->createLayer(tr("Layer 1"));
    layer->background()->setColor(Qt::white);
    return res;
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
    // XXX We should also copy canvas properties

    // Block signals
    blockSignals(true);

    // Reset to default
    clear(true);

    // Copy layers
    foreach(Layer * layer, other->layers_)
        addLayer_(layer->clone(), true);
    activeLayerIndex_ = other->activeLayerIndex_;

    // Reset hovered
    indexHovered_ = -1;

    // Unblock signals
    blockSignals(false);

    // Emit signals
    emit needUpdatePicking();
    emitChanged();
    emit selectionChanged();
    emit layerAttributesChanged();
}

void Scene::clear(bool silent)
{
    // Block signals
    blockSignals(true);

    // Delete all layers
    foreach(Layer * layer, layers_)
        delete layer;
    layers_.clear();
    activeLayerIndex_ = -1;

    // XXX Shouldn't this clear left/top/width/height too?

    // Unblock signals
    blockSignals(false);

    // Emit signals
    if(!silent)
    {
        emitChanged();
        emit needUpdatePicking();
        emit selectionChanged();
        emit layerAttributesChanged();
    }
}

Scene::~Scene()
{
    clear(true);
}

// ----------------------- Save and Load -------------------------

void Scene::save(QTextStream & out)
{
    Q_UNUSED(out);

    // XXX Deprecated
    /*
    out << Save::newField("SceneObjects");
    out << "\n" << Save::indent() << "[";
    Save::incrIndent();
    foreach(Layer * layer, layers_)
    {
        out << Save::openCurlyBrackets();
        layer->vac()->save(out);
        out << Save::closeCurlyBrackets();
    }
    Save::decrIndent();
    out << "\n" << Save::indent() << "]";
    */
}

void Scene::exportSVG(Time t, QTextStream & out)
{
    // Export Layers
    foreach(Layer * layer, layers_)
    {
        layer->background()->exportSVG(
            t.frame(), out, left(), top(), width(), height());
        layer->exportSVG(t, out);
    }
}

void Scene::read(QTextStream & in)
{
    Q_UNUSED(in);

    // XXX Deprecated
    /*
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
    */
}

void Scene::writeAllLayers(XmlStreamWriter & xml)
{
    foreach(Layer * layer, layers_)
    {
        xml.writeStartElement("layer");
        layer->write(xml);
        xml.writeEndElement();
    }
}

void Scene::readOneLayer(XmlStreamReader & xml)
{
    // Precondition: XML element "layer" just opened

    // XXX Remove this
    blockSignals(true);

    Layer * layer = new Layer();
    layer->read(xml);
    addLayer_(layer, true);

    // XXX Remove these: only emit once after finishing to read the file
    blockSignals(false);
    emit needUpdatePicking();
    emit changed();
    emit selectionChanged();
    emit layerAttributesChanged();
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
    foreach(Layer * layer, layers_)
    {
        layer->background()->relativeRemap(oldDir, newDir);
    }
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
    foreach(Layer * layer, layers_)
    {
        layer->draw(time, viewSettings);
    }
}

void Scene::drawPick(Time time, ViewSettings & viewSettings)
{
    // Find which layer to pick
    Layer * layer = activeLayer();
    int index = -1;
    for(int i=0; i<layers_.size(); i++)
    {
        if (layers_[i] == layer)
        {
            index = i;
            break;
        }
    }

    // Draw picking information
    if (index >= 0)
    {
        Picking::setIndex(index);
        layer->drawPick(time, viewSettings);
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
    foreach(Layer * layer, layers_)
        layer->deselectAll(time);
}

void Scene::deselectAll()
{
    foreach(Layer * layer, layers_)
        layer->deselectAll();
}

void Scene::invertSelection()
{
    foreach(Layer * layer, layers_)
        layer->invertSelection();
}

void Scene::selectAllInFrame()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->selectAllAtTime(global()->activeTime());
    }
}

void Scene::selectAllInAnimation()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->selectAll();
    }
}

void Scene::selectConnected()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->selectConnected();
    }
}

void Scene::selectClosure()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->selectClosure();
    }
}

void Scene::selectVertices()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->selectVertices();
    }
}

void Scene::selectEdges()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->selectEdges();
    }
}

void Scene::selectFaces()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->selectFaces();
    }
}

void Scene::deselectVertices()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->deselectVertices();
    }
}

void Scene::deselectEdges()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->deselectEdges();
    }
}

void Scene::deselectFaces()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->deselectFaces();
    }
}

void Scene::selectKeyCells()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->selectKeyCells();
    }
}


void Scene::selectInbetweenCells()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->selectInbetweenCells();
    }
}


void Scene::deselectKeyCells()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->deselectKeyCells();
    }
}


void Scene::deselectInbetweenCells()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->deselectInbetweenCells();
    }
}

void Scene::selectKeyVertices()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->selectKeyVertices();
    }
}

void Scene::selectKeyEdges()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->selectKeyEdges();
    }
}

void Scene::selectKeyFaces()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->selectKeyFaces();
    }
}

void Scene::deselectKeyVertices()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->deselectKeyVertices();
    }
}

void Scene::deselectKeyEdges()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->deselectKeyEdges();
    }
}

void Scene::deselectKeyFaces()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->deselectKeyFaces();
    }
}

void Scene::selectInbetweenVertices()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->selectInbetweenVertices();
    }
}

void Scene::selectInbetweenEdges()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->selectInbetweenEdges();
    }
}

void Scene::selectInbetweenFaces()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->selectInbetweenFaces();
    }
}

void Scene::deselectInbetweenVertices()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->deselectInbetweenVertices();
    }
}

void Scene::deselectInbetweenEdges()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->deselectInbetweenEdges();
    }
}

void Scene::deselectInbetweenFaces()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->deselectInbetweenFaces();
    }
}

void Scene::keyPressEvent(QKeyEvent *event)
{
    event->ignore();
}

void Scene::keyReleaseEvent(QKeyEvent *event)
{
    event->ignore();
}

void Scene::addLayer_(Layer * layer, bool silent)
{
    layers_ << layer;
    if (activeLayerIndex_ < 0) {
        activeLayerIndex_ = 0;
    }

    connect(layer, SIGNAL(changed()), this, SIGNAL(changed()));
    connect(layer, SIGNAL(checkpoint()), this, SIGNAL(checkpoint()));
    connect(layer, SIGNAL(needUpdatePicking()), this, SIGNAL(needUpdatePicking()));
    connect(layer, SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()));
    connect(layer, SIGNAL(layerAttributesChanged()), this, SIGNAL(layerAttributesChanged()));

    if(!silent)
    {
        emitChanged();
        emit needUpdatePicking();
        emit layerAttributesChanged();
    }
}

void Scene::populateToolBar(QToolBar * toolBar)
{
    VectorAnimationComplex::VAC::populateToolBar(toolBar, this);
}

void Scene::deleteSelectedCells()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->deleteSelectedCells();
    }
}

void Scene::test()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->test();
    }
}

void Scene::smartDelete()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->smartDelete();
    }
}

int Scene::numLayers() const
{
    return layers_.size();
}

Layer* Scene::layer(int i) const
{
    if (0 <= i && i < numLayers())
    {
        return layers_[i];
    }
    else
    {
        return nullptr;
    }
}

void Scene::setActiveLayer(int i)
{
    if(i != activeLayerIndex_ && 0 <= i && i < numLayers())
    {
        deselectAll();

        activeLayerIndex_ = i;

        emitChanged();
        emit needUpdatePicking();
        emit layerAttributesChanged();
    }
}

Layer * Scene::activeLayer() const
{
    return layer(activeLayerIndex_);
}

int Scene::activeLayerIndex() const
{
    return activeLayerIndex_;
}

VectorAnimationComplex::VAC * Scene::activeVAC() const
{
    Layer * layer = activeLayer();
    return layer ? layer->vac() : nullptr;
}

Background * Scene::activeBackground() const
{
    Layer * layer = activeLayer();
    return layer ? layer->background() : nullptr;
}

Layer * Scene::createLayer()
{
    return createLayer(tr("Layer %1").arg(numLayers() + 1));
}

Layer * Scene::createLayer(const QString & name)
{
    // Create new layer, add it on top for now
    Layer * layer = new Layer(name);
    addLayer_(layer, true);

    // Move above active layer, or keep last if no active layer
    int newActiveLayerIndex = layers_.size() - 1;
    if (0 <= activeLayerIndex_ && activeLayerIndex_ < newActiveLayerIndex - 1)
    {
        newActiveLayerIndex = activeLayerIndex_ + 1;
        for (int i = layers_.size() - 1; i > newActiveLayerIndex; --i)
        {
            layers_[i] = layers_[i-1];
        }
        layers_[newActiveLayerIndex] = layer;
    }

    // Set as active
    activeLayerIndex_ = newActiveLayerIndex;

    // Emit signals
    emitChanged();
    emit needUpdatePicking();
    emit layerAttributesChanged();

    return layer;
}

void Scene::moveActiveLayerUp()
{
    int i = activeLayerIndex_;
    if(0 <= i && i < numLayers() - 1)
    {
        // Swap out layers
        int j = i + 1;
        std::swap(layers_[i], layers_[j]);

        // Set new active index.
        // Note: we don't call setActiveLayer(j) to avoid calling deselectAll().
        activeLayerIndex_ = j;

        // Emit signals. Note: it may be tempting to think that updating
        // picking is unnecessary (because the pickable cells haven't moved),
        // however the picking image data contains the layer index which has
        // changed, and therefore this picking image needs to be re-rendered.
        emitChanged();
        emit needUpdatePicking();
        emit layerAttributesChanged();
    }
}

void Scene::moveActiveLayerDown()
{
    int i = activeLayerIndex_;
    if(0 < i && i < numLayers())
    {
        // Swap out layers
        int j = i - 1;
        std::swap(layers_[i], layers_[j]);

        // Set new active index.
        // Note: we don't call setActiveLayer(j) to avoid calling deselectAll().
        activeLayerIndex_ = j;

        // Emit signals. Note: it may be tempting to think that updating
        // picking is unnecessary (because the pickable cells haven't moved),
        // however the picking image data contains the layer index which has
        // changed, and therefore this picking image needs to be re-rendered.
        emitChanged();
        emit needUpdatePicking();
        emit layerAttributesChanged();
    }
}

void Scene::destroyActiveLayer()
{
    int i = activeLayerIndex_;
    if(0 <= i && i < numLayers())
    {
        deselectAll();

        Layer * toBeDestroyedLayer = layers_[i];
        layers_.removeAt(i);

        // Set as active the layer below, unless it was the bottom-most layer
        // or the only layer in the scene.
        if (numLayers() == 0)
        {
            // no more layers
            activeLayerIndex_ = -1;
        }
        else if (activeLayerIndex_ == 0)
        {
            // was the bottom-most layer
            activeLayerIndex_ = 0;
        }
        else
        {
            // had layer below
            --activeLayerIndex_;
        }

        delete toBeDestroyedLayer;

        emitChanged();
        emit needUpdatePicking();
        emit layerAttributesChanged();
    }
}

VectorAnimationComplex::InbetweenFace * Scene::createInbetweenFace()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        return layer->vac()->newInbetweenFace(
                    QList<VectorAnimationComplex::AnimatedCycle>(),
                    QSet<VectorAnimationComplex::KeyFace*>(),
                    QSet<VectorAnimationComplex::KeyFace*>());
    }
    else
    {
        return nullptr;
    }
}

void Scene::cut(VectorAnimationComplex::VAC* & clipboard)
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->cut(clipboard);
    }
}

void Scene::copy(VectorAnimationComplex::VAC* & clipboard)
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->copy(clipboard);
    }
}

void Scene::paste(VectorAnimationComplex::VAC* & clipboard)
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->paste(clipboard);
    }
}

void Scene::motionPaste(VectorAnimationComplex::VAC* & clipboard)
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->motionPaste(clipboard);
    }
}

void Scene::createFace()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->createFace();
    }
}

void Scene::addCyclesToFace()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->addCyclesToFace();
    }
}

void Scene::removeCyclesFromFace()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->removeCyclesFromFace();
    }
}

void Scene::changeColor()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->changeColor();
    }
}
void Scene::raise()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->raise();
    }
}

void Scene::lower()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->lower();
    }
}

void Scene::raiseToTop()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->raiseToTop();
    }
}

void Scene::lowerToBottom()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->lowerToBottom();
    }
}

void Scene::altRaise()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->altRaise();
    }
}

void Scene::altLower()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->altLower();
    }
}

void Scene::altRaiseToTop()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->altRaiseToTop();
    }
}

void Scene::altLowerToBottom()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->altLowerToBottom();
    }
}

void Scene::changeEdgeWidth()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->changeEdgeWidth();
    }
}

void Scene::glue()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->glue();
    }
}

void Scene::unglue()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->unglue();
    }
}

void Scene::uncut()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->uncut();
    }
}

void Scene::inbetweenSelection()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->inbetweenSelection();
    }
}

void Scene::keyframeSelection()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->keyframeSelection();
    }
}

void Scene::resetCellsToConsiderForCutting()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->resetCellsToConsiderForCutting();
    }
}

void Scene::updateCellsToConsiderForCutting()
{
    Layer * layer = activeLayer();
    if(layer)
    {
        layer->vac()->updateCellsToConsiderForCutting();
    }
}
}

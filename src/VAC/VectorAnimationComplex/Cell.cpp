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

#include <QApplication>
#include "../OpenGL.h"
#include <QtDebug>
#include <QTextStream>
#include "../Picking.h"
#include "../DevSettings.h"
#include "../Global.h"

#include "Cell.h"

#include "VAC.h"

#include "KeyCell.h"
#include "KeyVertex.h"
#include "KeyEdge.h"
#include "KeyFace.h"
#include "InbetweenCell.h"
#include "InbetweenVertex.h"
#include "InbetweenEdge.h"
#include "InbetweenFace.h"
#include "Algorithms.h"

#include "../ViewSettings.h"
#include "../View3DSettings.h"

#include "../SaveAndLoad.h"

#include "../XmlStreamReader.h"
#include "../XmlStreamWriter.h"

#include "../CssColor.h"

namespace VectorAnimationComplex
{

Cell::Cell(VAC * vac) :
    vac_(vac), id_(-1),
    isHovered_(0), isSelected_(0)
{
    colorHighlighted_[0] = 1;
    colorHighlighted_[1] = 0.7;
    colorHighlighted_[2] = 0.7;
    colorHighlighted_[3] = 1;

    colorSelected_[0] = 1;
    colorSelected_[1] = 0;
    colorSelected_[2] = 0;
    colorSelected_[3] = 1;

    color_[0] = 0;
    color_[1] = 0;
    color_[2] = 0;
    color_[3] = 1;
}

Cell * Cell::toCell()                       { return this; }
KeyCell * Cell::toKeyCell()                 { return dynamic_cast<KeyCell*>(this); }
InbetweenCell * Cell::toInbetweenCell()     { return dynamic_cast<InbetweenCell*>(this); }
VertexCell * Cell::toVertexCell()           { return dynamic_cast<VertexCell*>(this); }
EdgeCell * Cell::toEdgeCell()               { return dynamic_cast<EdgeCell*>(this); }
FaceCell * Cell::toFaceCell()               { return dynamic_cast<FaceCell*>(this); }
KeyVertex * Cell::toKeyVertex()             { return dynamic_cast<KeyVertex*>(this); }
KeyEdge * Cell::toKeyEdge()                 { return dynamic_cast<KeyEdge*>(this); }
KeyFace * Cell::toKeyFace()                 { return dynamic_cast<KeyFace*>(this); }
InbetweenVertex * Cell::toInbetweenVertex() { return dynamic_cast<InbetweenVertex*>(this); }
InbetweenEdge * Cell::toInbetweenEdge()     { return dynamic_cast<InbetweenEdge*>(this); }
InbetweenFace * Cell::toInbetweenFace()     { return dynamic_cast<InbetweenFace*>(this); }

Cell::~Cell()
{
}
void Cell::destroy()
{
    vac()->deleteCell(this);
}
void Cell::destroyStar()
{
    while(!star().isEmpty())
        (*star().begin())->destroy();
}
void Cell::informBoundaryImGettingDestroyed()
{
    removeMeFromStarOfBoundary_();
}

void Cell::addObserver(CellObserver * observer)
{
    observers_.insert(observer);
}

void Cell::removeObserver(CellObserver * observer)
{
    observers_.remove(observer);
}

int Cell::dimension()
{
    if (toKeyVertex())
        return 0;
    else if (toKeyEdge() || toInbetweenVertex())
        return 1;
    else if (toKeyFace() || toInbetweenEdge())
        return 2;
    else
        return 3;
}

// Update cell boundary
void Cell::updateBoundary_preprocess()
{
    removeMeFromStarOfBoundary_();
}

void Cell::updateBoundary_postprocess()
{
    addMeToStarOfBoundary_();
}

void Cell::updateBoundary(KeyVertex * oldVertex, KeyVertex * newVertex)
{
    updateBoundary_preprocess();
    updateBoundary_impl(oldVertex,newVertex);
    updateBoundary_postprocess();
}

void Cell::updateBoundary(const KeyHalfedge & oldHalfedge, const KeyHalfedge & newHalfedge)
{
    updateBoundary_preprocess();
    updateBoundary_impl(oldHalfedge,newHalfedge);
    updateBoundary_postprocess();
}

void Cell::updateBoundary(KeyEdge * oldEdge, const KeyEdgeList & newEdges)
{
    updateBoundary_preprocess();
    updateBoundary_impl(oldEdge,newEdges);
    updateBoundary_postprocess();
}

void Cell::updateBoundary_impl(KeyVertex * , KeyVertex * )    {}
void Cell::updateBoundary_impl(const KeyHalfedge & , const KeyHalfedge & ) {}
void Cell::updateBoundary_impl(KeyEdge * , const KeyEdgeList & ) {}


Cell::Cell(Cell * other)
{
    vac_ = other->vac_;
    id_ = other->id_;
    //isHighlighted_ = other->isHighlighted_;
    isHovered_ = 0;
    isSelected_ = other->isSelected_;
    // this sounds like a bad architecture: other->colorHighlighted
    // should not be zero. Kept like this for now anyway
    if (other->colorHighlighted_)
        for (int i=0; i<4; i++)
            colorHighlighted_[i] = other->colorHighlighted_[i];
    if (other->colorSelected_)
        for (int i=0; i<4; i++)
            colorSelected_[i] = other->colorSelected_[i];
    if (other->color_)
        for (int i=0; i<4; i++)
            color_[i] = other->color_[i];
    spatialStar_ = other->spatialStar_;
    temporalStarBefore_ = other->temporalStarBefore_;
    temporalStarAfter_ = other->temporalStarAfter_;
}

void Cell::remapPointers(VAC * newVAC)
{
    vac_ = newVAC;

    {
        CellSet old = spatialStar_;
        spatialStar_.clear();
        auto it = old.begin();
        auto itEnd = old.end();
        for(; it != itEnd; ++it)
            spatialStar_ << newVAC->getCell((*it)->id());
    }
    {
        CellSet old = temporalStarBefore_;
        temporalStarBefore_.clear();
        auto it = old.begin();
        auto itEnd = old.end();
        for(; it != itEnd; ++it)
            temporalStarBefore_ << newVAC->getCell((*it)->id());
    }
    {
        CellSet old = temporalStarAfter_;
        temporalStarAfter_.clear();
        auto it = old.begin();
        auto itEnd = old.end();
        for(; it != itEnd; ++it)
            temporalStarAfter_ << newVAC->getCell((*it)->id());
    }
}

Cell * Cell::getCell(int id)
{
    return vac()->getCell(id);
}

QColor Cell::color() const
{
    QColor res;
    res.setRgbF(color_[0],color_[1],color_[2],color_[3]);
    return res;
}

void Cell::setColor(const QColor & c)
{
    color_[0] = c.redF();
    color_[1] = c.greenF();
    color_[2] = c.blueF();
    color_[3] = c.alphaF();
}

bool Cell::isHighlighted() const
{
    if(isHovered())
    {
        if((global()->toolMode() == Global::SELECT))
        {
            Qt::KeyboardModifiers keys = global()->keyboardModifiers();

            if(isSelected())
            {
                if((keys & Qt::AltModifier))
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                if( (keys & Qt::AltModifier) && !(keys & Qt::ShiftModifier) )
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
        }
        else if((global()->toolMode() == Global::SKETCH))
        {
            return false;
        }
        else if((global()->toolMode() == Global::EDIT_CANVAS_SIZE))
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return false;
    }
}



void Cell::glColorTopology_()
{
    if(isHighlighted())
        glColor4dv(colorHighlighted_);
    else if(isSelected() && global()->toolMode() == Global::SELECT)
        glColor4dv(colorSelected_);
    else
    {
        bool inbetweenOutlineDifferentColor = true;
        if(inbetweenOutlineDifferentColor)
        {
            if(toKeyVertex())
                glColor4d(0,0.165,0.514,1);
            else if(toKeyEdge())
                glColor4d(0.18,0.60,0.90,1);
            else if(toKeyFace())
                glColor4d(0.75,0.90,1.00,1);
            else if(toInbetweenVertex())
                glColor4d(0.12,0.34,0,1);
            else if(toInbetweenEdge())
                glColor4d(0.47,0.72,0.40,1);
            else if(toInbetweenFace())
                glColor4d(0.94,1.00,0.91,1);
            else // shouldn't happen
                glColor4d(0,0,0,1);
        }
        else
        {
            if(toVertexCell())
                glColor4d(0,0.165,0.514,1);
            else if(toEdgeCell())
                glColor4d(0.18,0.60,0.90,1);
            else // shouldn't happen
                glColor4d(0,0,0,1);
        }
    }
}

QColor Cell::getColor(Time /*time*/, ViewSettings & /*viewSettings*/) const
{
    QColor res;
    res.setRedF(color_[0]);
    res.setGreenF(color_[1]);
    res.setBlueF(color_[2]);
    res.setAlphaF(color_[3]);
    return res;
}

void Cell::glColor_(Time time, ViewSettings & viewSettings)
{
    if(global()->displayMode() == Global::ILLUSTRATION_OUTLINE && !toFaceCell())
    {
        QColor c = getColor(time, viewSettings);
        glColor4d(c.redF(), c.greenF(), c.blueF(), c.alphaF());
    }
    else if(isHighlighted())
        glColor4dv(colorHighlighted_);
    else if(isSelected() && global()->toolMode() == Global::SELECT)
        glColor4dv(colorSelected_);
    else
    {
        QColor c = getColor(time, viewSettings);
        glColor4d(c.redF(), c.greenF(), c.blueF(), c.alphaF());
    }
}

void Cell::glColor3D_()
{
    if(global()->displayMode() == Global::ILLUSTRATION_OUTLINE && !toFaceCell())
    {
        glColor4dv(color_);
    }
    else if(isHighlighted())
        glColor4dv(colorHighlighted_);
    else if(isSelected() && global()->toolMode() == Global::SELECT)
        glColor4dv(colorSelected_);
    else
    {
        glColor4dv(color_);
    }
}



////////////////////////////     Draw    ///////////////////////////////

void Cell::draw(Time time, ViewSettings & viewSettings)
{
    if (!exists(time))
        return;

    glColor_(time, viewSettings);
    drawRaw(time, viewSettings);
}

void Cell::drawRaw(Time time, ViewSettings & /*viewSettings*/)
{
    triangles(time).draw();
}

void Cell::drawPick(Time time, ViewSettings & viewSettings)
{
    if (!isPickable(time))
    {
        return;
    }
    else
    {
        Picking::glColor(id());
        drawPickCustom(time, viewSettings);
    }
}

void Cell::drawPickCustom(Time time, ViewSettings & viewSettings)
{
    drawRaw(time, viewSettings);
}



/////////////////////////     Draw Topology   /////////////////////////////

void Cell::drawTopology(Time time, ViewSettings & viewSettings)
{
    if (!exists(time))
        return;

    glColorTopology_();
    drawRawTopology(time, viewSettings);
}

void Cell::drawRawTopology(Time time, ViewSettings & /*viewSettings*/)
{
    triangles(time).draw();
}

void Cell::drawPickTopology(Time time, ViewSettings & viewSettings)
{
    if (!isPickable(time))
    {
        return;
    }
    else
    {
        Picking::glColor(id());
        drawPickTopologyCustom(time, viewSettings);
    }
}

void Cell::drawPickTopologyCustom(Time time, ViewSettings & viewSettings)
{
    drawRawTopology(time, viewSettings);
}



/////////////////////////     Draw 3D   /////////////////////////////

void Cell::draw3D(View3DSettings & viewSettings)
{
    glColor3D_();
    this->drawRaw3D(viewSettings);
}

void Cell::drawRaw3D(View3DSettings & /*viewSettings*/)
{
}

void Cell::drawPick3D(View3DSettings & /*viewSettings*/)
{
}

bool Cell::isPickable(Time time) const
{
    if (!exists(time))
        return false;
    else
        return isPickableCustom(time);
}

bool Cell::isPickableCustom(Time /*time*/) const
{
    return false;
}

// Topological Navigation Information
// ------------ Boundary ------------
CellSet Cell::boundary() const
{
    CellSet res = this->spatialBoundary();
    CellSet other = this->temporalBoundary();
    res.unite(other);
    return res;
}
CellSet Cell::spatialBoundary() const { return CellSet(); }
CellSet Cell::spatialBoundary(Time t) const
{
    CellSet res;
    const auto& spatialBoundaryCells = this->spatialBoundary();
    for(Cell * obj: spatialBoundaryCells)
        if(obj->exists(t))
            res << obj;
    return res;
}
KeyCellSet Cell::temporalBoundary() const
{
    KeyCellSet res = this->beforeCells();
    res.unite(this->afterCells());
    return res;
}
KeyCellSet Cell::beforeCells() const { return KeyCellSet(); }
KeyCellSet Cell::afterCells() const { return KeyCellSet(); }
// -------------- Star --------------
CellSet Cell::star() const
{
    CellSet res = this->spatialStar();
    CellSet other = this->temporalStar();
    res.unite(other);
    return res;
}
CellSet Cell::spatialStar() const
{
    return spatialStar_;
}
CellSet Cell::spatialStar(Time t) const
{
    if (exists(t))
        return this->spatialStar();
    else
        return CellSet();
    // this comes from the property that the spatial star
    // of the cell does not depend on time, since by construction
    // the temporal boundary of cells is instant
    // -> (as far as I remember... would be good to check again)
}
CellSet Cell::temporalStar() const
{
    CellSet res = this->temporalStarBefore();
    res.unite(this->temporalStarAfter());
    return res;
}
CellSet Cell::temporalStarBefore() const
{
    return temporalStarBefore_;
}
CellSet Cell::temporalStarAfter() const
{
    return temporalStarAfter_;
}

// ---------- Neighbourhood ---------
CellSet Cell::neighbourhood()  const
{
    CellSet res = this->boundary();
    res.unite(this->star());
    return res;
}
CellSet Cell::spatialNeighbourhood() const
{
    CellSet res = this->spatialBoundary();
    res.unite(this->spatialStar());
    return res;
}
CellSet Cell::spatialNeighbourhood(Time t)  const
{
    CellSet res = this->spatialBoundary(t);
    res.unite(this->spatialStar(t));
    return res;
}
CellSet Cell::temporalNeighbourhood() const
{
    CellSet res = this->temporalBoundary();
    CellSet other = this->temporalStar();
    res.unite(other);
    return res;
}
CellSet Cell::temporalNeighbourhoodBefore() const
{
    CellSet res = this->beforeCells();
    CellSet other = this->temporalStarBefore();
    res.unite(other);
    return res;
}
CellSet Cell::temporalNeighbourhoodAfter() const
{
    CellSet res = this->afterCells();
    CellSet other = this->temporalStarAfter();
    res.unite(other);
    return res;
}
// -- Modifying star of boundary --
void Cell::addMeToStarOfBoundary_()
{
    const auto& spatialBoundaryCells = spatialBoundary();
    for(Cell * c: spatialBoundaryCells)
        addMeToSpatialStarOf_(c);

    const auto& beforeKeyCells = beforeCells();
    for(KeyCell * c: beforeKeyCells)
        addMeToTemporalStarAfterOf_(c);

    const auto& afterKeyCells = afterCells();
    for(KeyCell * c: afterKeyCells)
        addMeToTemporalStarBeforeOf_(c);
}

void Cell::removeMeFromStarOfBoundary_()
{
    const auto& spatialBoundaryCells = spatialBoundary();
    for(Cell * c: spatialBoundaryCells)
        removeMeFromSpatialStarOf_(c);

    const auto& beforeKeyCells = beforeCells();
    for(KeyCell * c: beforeKeyCells)
        removeMeFromTemporalStarAfterOf_(c);

    const auto& afterKeyCells = afterCells();
    for(KeyCell * c: afterKeyCells)
        removeMeFromTemporalStarBeforeOf_(c);
}

void Cell::removeMeFromStarOf_(Cell * c)
{
    removeMeFromSpatialStarOf_(c);
    removeMeFromTemporalStarBeforeOf_(c);
    removeMeFromTemporalStarAfterOf_(c);
}

void Cell::addMeToSpatialStarOf_(Cell * c)
{
    c->spatialStar_ << this;
}
void Cell::addMeToTemporalStarBeforeOf_(Cell *c)
{
    c->temporalStarBefore_ << this;
}
void Cell::addMeToTemporalStarAfterOf_(Cell *c)
{
    c->temporalStarAfter_ << this;

}
void Cell::removeMeFromSpatialStarOf_(Cell * c)
{
    c->spatialStar_.remove(this);
}
void Cell::removeMeFromTemporalStarBeforeOf_(Cell *c)
{
    c->temporalStarBefore_.remove(this);
}
void Cell::removeMeFromTemporalStarAfterOf_(Cell * c)
{
    c->temporalStarAfter_.remove(this);
}

void Cell::save(QTextStream & out)
{
    // properties shared by all objects
    out << Save::newField("Type") << stringType();
    out << Save::newField("ID") << id();
    out << Save::newField("Color")
        << color_[0] << " "
        << color_[1] << " "
        << color_[2] << " "
        << color_[3];

    // specific properties
    save_(out);

}

void Cell::save_(QTextStream & /*out*/)
{
}

void Cell::exportSVG(Time /*t*/, QTextStream & /*out*/)
{
}

Cell * Cell::read1stPass(VAC * vac, QTextStream & in)
{
    Field field;
    QString type;
    in >> field >> type;

    if(type == "Vertex" || type == "KeyVertex" || type == "InstantVertex" )
    {
        return KeyVertex::Read1stPass::create(vac, in);
    }
    else if(type == "Edge" || type == "KeyEdge" || type == "InstantEdge" )
    {
        return KeyEdge::Read1stPass::create(vac, in);
    }
    else if(type == "Face" || type == "KeyFace" || type == "InstantFace" )
    {
        return KeyFace::Read1stPass::create(vac, in);
    }
    else if(type == "InbetweenVertex" || type == "SpacetimeVertex" )
    {
        return InbetweenVertex::Read1stPass::create(vac, in);
    }
    else if(type == "InbetweenEdge" || type == "SpacetimeEdge" )
    {
        return InbetweenEdge::Read1stPass::create(vac, in);
    }
    else if(type == "InbetweenFace" || type == "SpacetimeFace" )
    {
        return InbetweenFace::Read1stPass::create(vac, in);
    }
    else
        return 0;

}
void Cell::read2ndPass()
{
    // transform IDs to pointers via vac()->getCell(id);
}

// Note: with this constructor, it is the VAC's responsibility
// to insert it in its list of objects.
Cell::Cell(VAC * vac, QTextStream & in) :
    vac_(vac), id_(-1),
    isHovered_(0), isSelected_(0)
{
    Field field;
    in >> field >> id_;

    colorHighlighted_[0] = 1;
    colorHighlighted_[1] = 0.7;
    colorHighlighted_[2] = 0.7;
    colorHighlighted_[3] = 1;

    colorSelected_[0] = 1;
    colorSelected_[1] = 0;
    colorSelected_[2] = 0;
    colorSelected_[3] = 1;

    double r, g, b, a;
    in >> field >> r >> g >> b >> a;
    QColor c;
    c.setRedF(r);
    c.setGreenF(g);
    c.setBlueF(b);
    c.setAlphaF(a);
    setColor(c);
}

void Cell::write(XmlStreamWriter & xml) const
{
    xml.writeStartElement(xmlType_());
    xml.writeAttribute("id", QString().setNum(id()));
    write_(xml);
    CssColor cssColor(color_);
    xml.writeAttribute("color", cssColor.toString());
    xml.writeEndElement();
}

void Cell::write_(XmlStreamWriter & /*xml*/) const
{

}

QString Cell::xmlType_() const
{
    return "cell";
}

Cell::Cell(VAC * vac, XmlStreamReader & xml) :
    vac_(vac), id_(-1),
    isHovered_(0), isSelected_(0)
{
    id_ = xml.attributes().value("id").toInt();

    if(xml.attributes().hasAttribute("color"))
    {
        CssColor c(xml.attributes().value("color").toString());
        color_[0] = c.rF();
        color_[1] = c.gF();
        color_[2] = c.bF();
        color_[3] = c.aF();
    }
    else
    {
        color_[0] = 0;
        color_[1] = 0;
        color_[2] = 0;
        color_[3] = 1;
    }

    colorHighlighted_[0] = 1;
    colorHighlighted_[1] = 0.7;
    colorHighlighted_[2] = 0.7;
    colorHighlighted_[3] = 1;

    colorSelected_[0] = 1;
    colorSelected_[1] = 0;
    colorSelected_[2] = 0;
    colorSelected_[3] = 1;
}

bool Cell::check() const
{
    // check incident cells share the same VAC

    // check that the cell belongs to its VAC
    if(!vac()->checkContains(this))
        return false;

    // other type-specific checks
    return check_();
}


//###################################################################
//                         GEOMETRY
//###################################################################

const Triangles & Cell::triangles(Time t) const
{
    // Get cache key
    int key = std::floor(t.floatTime() * 60 + 0.5);

    // Compute triangles if not yet cached
    if(!triangles_.contains(key))
        triangulate_(t, triangles_[key]);

    // Return cached triangles
    return triangles_[key];
}

const BoundingBox & Cell::boundingBox(Time t) const
{
    // Get cache key
    int key = std::floor(t.floatTime() * 60 + 0.5);

    // Compute bounding box if not yet cached
    if(!boundingBoxes_.contains(key))
        boundingBoxes_[key] = triangles(t).boundingBox();

    // Return cached bounding box
    return boundingBoxes_[key];
}

const BoundingBox & Cell::outlineBoundingBox(Time t) const
{
    // Get cache key
    int key = std::floor(t.floatTime() * 60 + 0.5);

    // Compute bounding box if not yet cached
    if(!outlineBoundingBoxes_.contains(key))
        computeOutlineBoundingBox_(t, outlineBoundingBoxes_[key]);

    // Return cached bounding box
    return outlineBoundingBoxes_[key];
}

bool Cell::intersects(Time t, const BoundingBox & bb) const
{
    return triangles(t).intersects(bb);
}

void Cell::processGeometryChanged_()
{
    CellSet toClearCells = geometryDependentCells_();
    for(Cell * cell: toClearCells)
        cell->clearCachedGeometry_();
}

void Cell::clearCachedGeometry_()
{
    triangles_.clear();
    boundingBoxes_.clear();
    outlineBoundingBoxes_.clear();
}

// XXX this could be cached, it is called many times during
// drag and drop and affine transform while not changing
CellSet Cell::geometryDependentCells_()
{
    CellSet res;
    res << this;

    // Because of the Catmull-Rom scheme, need to reach further
    KeyVertex * keyVertex = toKeyVertex();
    if(keyVertex)
    {
        CellSet beforeVertices = keyVertex->beforeVertices();
        CellSet afterVertices = keyVertex->afterVertices();
        res.unite(beforeVertices);
        res.unite(afterVertices);
    }

    return Algorithms::fullstar(res);
}

}

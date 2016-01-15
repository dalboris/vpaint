// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "../OpenGL.h"

#include "VAC.h"

#include "KeyVertex.h"
#include "KeyEdge.h"
#include "KeyFace.h"
#include "InbetweenVertex.h"
#include "InbetweenEdge.h"
#include "InbetweenFace.h"

#include "Halfedge.h"
#include "ProperCycle.h"
#include "Cycle.h"
#include "ProperPath.h"
#include "Path.h"
#include "SmartKeyEdgeSet.h"

#include "Algorithms.h"
#include "CellObserver.h"

#include "EdgeSample.h"
#include "EdgeGeometry.h"
#include "Intersection.h"

#include "../GLUtils.h"
#include "../Timeline.h"
#include "../SaveAndLoad.h"
#include "../DevSettings.h"
#include "../Global.h"
#include "../MainWindow.h"
#include "../View.h"
#include "../Color.h"
#include "../Scene.h"

#include "../XmlStreamWriter.h"
#include "../XmlStreamReader.h"

#include <QPair>
#include <QtDebug>
#include <QApplication>
#include <QMessageBox>
#include <QStatusBar>
#include <QColorDialog>
#include <QInputDialog>

#define MYDEBUG 0

namespace VectorAnimationComplex
{


// unnamed namespace not to pollute global namespace with auxiliary functions
namespace
{

bool isCycleContainedInFace(const Cycle & cycle, const PreviewKeyFace & face)
{
    // Get edges involved in cycle
    KeyEdgeSet cycleEdges = cycle.cells();

    // Compute total length of edges
    double totalLength = 0;
    foreach (KeyEdge * edge, cycleEdges)
        totalLength += edge->geometry()->length();

    // Compute percentage of edges inside face, based on approximately N samples
    double N = 100;
    double ds = totalLength / N;
    double nInside = 0;
    double nOutside = 0;
    foreach (KeyEdge * edge, cycleEdges)
    {
        EdgeGeometry * geometry = edge->geometry();
        double L = geometry->length();
        for(double s=0; s<L; s+=ds)
        {
            Eigen::Vector2d p = geometry->pos2d(s);
            if(face.intersects(p[0],p[1]))
            {
                nInside++;
            }
            else
            {
                nOutside++;
            }
        }
    }
    if(nInside > nOutside)
        return true;
    else
        return false;
}

// return invalid cycle if not found
Cycle findClosestPlanarCycle(QSet<KeyEdge*> & potentialEdges,
                                    QMap<KeyEdge*,EdgeGeometry::ClosestVertexInfo> & distancesToEdges, double x, double y)
{
    while(!potentialEdges.isEmpty())
    {
        // Find closest potential edge
        KeyEdge * closestPotentialEdge = 0;
        EdgeGeometry::ClosestVertexInfo cvi;
        cvi.s = 0;
        cvi.d = std::numeric_limits<double>::max();
        foreach(KeyEdge * e, potentialEdges)
        {
            EdgeGeometry::ClosestVertexInfo cvi_e = distancesToEdges[e];
            if(cvi_e.d < cvi.d)
            {
                closestPotentialEdge = e;
                cvi = cvi_e;
            }
        }

        // Compute direction of halfedge
        Eigen::Vector2d der = closestPotentialEdge->geometry()->der(cvi.s);
        double cross = der[0] * (y - cvi.p.y()) - der[1] * (x - cvi.p.x());
        KeyHalfedge h(closestPotentialEdge, (cross>0 ? false : true) ); // Note: canvas is left-handed

        // Case where closestPotentialEdge is closed
        if(closestPotentialEdge->isClosed())
        {
            KeyEdgeSet edgeSet;
            edgeSet << closestPotentialEdge;
            Cycle cycle(edgeSet);
            if(cycle.isValid())
            {
                return cycle;
            }
            else
            {
                potentialEdges.remove(closestPotentialEdge);
            }
        }

        // Case where closestPotentialEdge is open
        else
        {
            // First halfedge of non-simple-cycle
            KeyHalfedge h0 = h;
            QList<KeyHalfedge> potentialPlanarCycle;
            potentialPlanarCycle << h;

            // Find the corresponding planar map cycle
            int maxIter = 2 * potentialEdges.size() + 2;
            bool foundPotentialPlanarCycle = false;
            for(int i=0; i<maxIter; i++)
            {
                // Find next halfedge in cycle
                h = h.next();

                // Check it has not already been rejected
                if(!potentialEdges.contains(h.edge))
                {
                    break;
                }

                // Test if cycle completed or not
                if(h == h0)
                {
                    // Cycle completed: leave loop
                    foundPotentialPlanarCycle = true;
                    break;
                }
                else
                {
                    // Cycle not completed: insert and iterate
                    potentialPlanarCycle << h;
                }
            }

            // If not found (maxIter reached or edge already rejected)
            if(!foundPotentialPlanarCycle)
            {
                foreach(KeyHalfedge he, potentialPlanarCycle)
                {
                    potentialEdges.remove(he.edge);
                }
            }
            else
            {
                Cycle cycle(potentialPlanarCycle);
                if(cycle.isValid())
                {
                    return cycle;
                }
                else
                {
                    foreach(KeyHalfedge he, potentialPlanarCycle)
                    {
                        potentialEdges.remove(he.edge);
                    }
                }
            }
        }
    }

    return Cycle();
}

bool addHoleToPaintedFace(QSet<KeyEdge*> & potentialHoleEdges,
                          PreviewKeyFace & toBePaintedFace,
                          QMap<KeyEdge*,EdgeGeometry::ClosestVertexInfo> & distancesToEdges,
                          double x, double y)
{
    while(!potentialHoleEdges.isEmpty())
    {
        // Find closest planar cycle
        Cycle cycle = findClosestPlanarCycle(potentialHoleEdges, distancesToEdges, x, y);

        // Returns directly if no planar cycle found, otherwise, proceed.
        if(!cycle.isValid())
        {
            return false;
        }

        // Remove potential edges
        KeyEdgeSet cycleEdges = cycle.cells();
        potentialHoleEdges.subtract(cycleEdges);

        // Create face from cycle for geometric query
        PreviewKeyFace face;
        face << cycle;

        // Check if the planar cycle should be added as a hole or not
        if(!face.intersects(x,y) && isCycleContainedInFace(cycle,toBePaintedFace))
        {
            // Add it as a hole
            toBePaintedFace << cycle;
            return true;
        }

    }// end while( !( found external boundary || isEmpty) )

    return false;
}

} // end of namespace



    // ----------------- Constructors & Destructors ----------------


void VAC::initNonCopyable()
{
    drawRectangleOfSelection_ = false;
    sketchedEdge_ = 0;
    hoveredFaceOnMousePress_ = 0;
    hoveredFaceOnMouseRelease_ = 0;
    sculptedEdge_ = 0;
    toBePaintedFace_ = 0;
    hoveredCell_ = 0;
    deselectAll();
    signalCounter_ = 0;
}

void VAC::initCopyable()
{
    maxID_ = -1;
    ds_ = 5.0;
    cells_.clear();
    zOrdering_.clear();
}


VAC::VAC() :
    SceneObject()
{
    initNonCopyable();
    initCopyable();
}

VAC::~VAC()
{
    deleteAllCells();
}

QString VAC::stringType()
{
    return "VectorAnimationComplex";
}


VAC * VAC::clone()
{
    // Create new Graph
    VAC * newVAC = new VAC();

    // Copy maxID
    newVAC->maxID_ = maxID_;

    // Copy sampling precision
    newVAC->ds_ = ds_;

    // Copy cells
    foreach(Cell * cell, cells_)
    {
        Cell * newCell = cell->clone();
        newVAC->cells_[newCell->id()] = newCell;
        newCell->setSelected(false);
        newCell->setHovered(false);
    }
    foreach(Cell * newCell, newVAC->cells_)
        newCell->remapPointers(newVAC);
    for(auto c: zOrdering_)
        newVAC->zOrdering_.insertLast(newVAC->getCell(c->id()));

    return newVAC;
}

// returns a map such as mp[oldID] = newID
QMap<int,int> VAC::import(VAC * other, bool selectImportedCells)
{
    QMap<int,int> res;

    // Create copy
    VAC * copyOfOther = other->clone();

    // Create copy of zOrdering, since removing cells
    // from copyOfOther invalidate iteration on zOrdering
    QList<Cell*> ordering;
    for(auto c: copyOfOther->zOrdering_)
    {
        ordering << c;
    }

    // Take ownership of all cells
    foreach(Cell * c, ordering)
    {
        int oldID = c->id();
        copyOfOther->removeCell_(c);
        insertCellLast_(c);
        if(selectImportedCells)
            addToSelection(c,false);
        res[oldID] = c->id();
    }

    // Delete copy of vac
    delete copyOfOther;

    return res;
}

VAC * VAC::subcomplex(const CellSet & subcomplexCells)
{
    // Get closure of cells
    CellSet cellsToKeep = Algorithms::closure(subcomplexCells);
    CellSet cellsToDelete = cells();
    cellsToDelete.subtract(cellsToKeep);
    QList<int> idToDelete;
    foreach(Cell * c, cellsToDelete)
        idToDelete << c->id();

    // Create new Graph
    VAC * newVAC = clone();

    // Delete all cells but the one to keep
    foreach (double id, idToDelete)
    {
        if(newVAC->cells_.contains(id))
            newVAC->deleteCell(newVAC->getCell(id));
    }

    // Return subcomplex
    return newVAC;
}

// ------------------------- Drawing ---------------------------

void VAC::drawSketchedEdge(Time time, ViewSettings & /*viewSettings*/) const
{
    if(!sketchedEdge_)
        return;

    if(time.frame() != timeInteractivity_.frame())
        return;

    if(sketchedEdge_->size() < 2)
        return;

    QColor edgeColor = global()->edgeColor();
    glColor4d(edgeColor.redF(),edgeColor.greenF(),edgeColor.blueF(),edgeColor.alphaF());

    // helper function
    auto getNormal = [] (double x1, double y1, double x2, double y2)
    {
        Eigen::Vector2d p1(x1, y1);
        Eigen::Vector2d p2(x2, y2);
        Eigen::Vector2d v = p2-p1;
        v.normalize();
        return Eigen::Vector2d(-v[1],v[0]);
    };

    // draw quad strip
    glBegin(GL_QUAD_STRIP);
    Eigen::Vector2d u = getNormal((*sketchedEdge_)[0].x(), (*sketchedEdge_)[0].y(),
            (*sketchedEdge_)[1].x(), (*sketchedEdge_)[1].y());
    Eigen::Vector2d p( (*sketchedEdge_)[0].x(), (*sketchedEdge_)[0].y() );
    Eigen::Vector2d A = p + (*sketchedEdge_)[0].width() * 0.5 * u;
    Eigen::Vector2d B = p - (*sketchedEdge_)[0].width() * 0.5 * u;
    glVertex2d(A[0], A[1]);
    glVertex2d(B[0], B[1]);
    p = Eigen::Vector2d( (*sketchedEdge_)[1].x(), (*sketchedEdge_)[1].y() );
    A = p + (*sketchedEdge_)[1].width() * 0.5 * u;
    B = p - (*sketchedEdge_)[1].width() * 0.5 * u;
    glVertex2d(A[0], A[1]);
    glVertex2d(B[0], B[1]);
    for(int i=2; i<(*sketchedEdge_).size(); i++)
    {
        Eigen::Vector2d u = getNormal((*sketchedEdge_)[i-1].x(), (*sketchedEdge_)[i-1].y(),
                (*sketchedEdge_)[i].x(), (*sketchedEdge_)[i].y());
        p = Eigen::Vector2d( (*sketchedEdge_)[i].x(), (*sketchedEdge_)[i].y() );
        A = p + (*sketchedEdge_)[i].width() * 0.5 * u;
        B = p - (*sketchedEdge_)[i].width() * 0.5 * u;
        glVertex2d(A[0], A[1]);
        glVertex2d(B[0], B[1]);
    }
    glEnd();

    // Start cap
    int n = 50;
    p = Eigen::Vector2d( sketchedEdge_->leftPos().x(), sketchedEdge_->leftPos().y() );
    double r = 0.5 * sketchedEdge_->leftPos().width();
    glBegin(GL_POLYGON);
    {
        for(int i=0; i<n; ++i)
        {
            double theta = 2 * (double) i * 3.14159 / (double) n ;
            glVertex2d(p.x() + r*std::cos(theta),p.y()+ r*std::sin(theta));
        }
    }
    glEnd();

    // End cap
    p = Eigen::Vector2d( sketchedEdge_->rightPos().x(), sketchedEdge_->rightPos().y() );
    r = 0.5 * sketchedEdge_->rightPos().width();
    glBegin(GL_POLYGON);
    {
        for(int i=0; i<n; ++i)
        {
            double theta = 2 * (double) i * 3.14159 / (double) n ;
            glVertex2d(p.x() + r*std::cos(theta),p.y()+ r*std::sin(theta));
        }
    }
    glEnd();
}

void VAC::drawTopologySketchedEdge(Time time, ViewSettings & viewSettings) const
{
    if(!sketchedEdge_)
        return;

    if(time.frame() != timeInteractivity_.frame())
        return;

    if(sketchedEdge_->size() < 2)
        return;

    glColor4d(0.18,0.60,0.90,1);
    glLineWidth(viewSettings.edgeTopologyWidth());
    glBegin(GL_LINE_STRIP);
    for(int i=0; i<(*sketchedEdge_).size(); i++)
        glVertex2d((*sketchedEdge_)[i].x(), (*sketchedEdge_)[i].y());
    glEnd();
}

void VAC::drawOneFrame3D(Time time, View3DSettings & viewSettings, ViewSettings & view2DSettings, bool drawAsTopo)
{
    // Translate to appropriate z value
    double z = viewSettings.zFromT(time);
    glPushMatrix();
    glScaled(1, -1, 1);
    glTranslated(0,0,z);

    glDisable(GL_LIGHTING);
    //glDisable(GL_DEPTH_TEST); // Responsability of caller to do this because
                                // sometimes it should be disabled, sometimes not
    double eps = 1.0e-2;
    for(auto c: zOrdering_)
    {
        if(drawAsTopo)
            c->drawTopology(time, view2DSettings);
        else
            c->draw(time, view2DSettings);
        if(c->exists(time))
            glTranslated(0,0,eps);
    }

    //glEnable(GL_DEPTH_TEST);

    glPopMatrix();
}

void VAC::drawAllFrames3D(View3DSettings & viewSettings, ViewSettings & view2DSettings)
{
    Timeline * timeline = global()->timeline();
    int firstFrame = timeline->firstFrame();
    int lastFrame = timeline->lastFrame();

    for(int i=lastFrame; i>=firstFrame; --i)
        drawOneFrame3D(Time(i), viewSettings, view2DSettings, viewSettings.drawFramesAsTopology());
}

void VAC::drawKeyCells3D(View3DSettings & viewSettings, ViewSettings & view2DSettings)
{
    QMap<int, QList<KeyCell *> > keyCellsOrderedAtFrame;
    for(auto c: zOrdering_)
    {
        KeyCell * kc = c->toKeyCell();
        if(kc)
        {
            int frame = kc->time().frame();
            keyCellsOrderedAtFrame[frame].append(kc);
        }
    }

    QMapIterator<int, QList<KeyCell *> > i(keyCellsOrderedAtFrame);
    while (i.hasNext())
    {
        i.next();

        // Get list of key frames at frame
        int frame = i.key();
        QList<KeyCell *> list = i.value();
        Time t = Time(frame);

        // Special case: don't draw if overlayed with current frame
        if(viewSettings.drawCurrentFrame() && ( t == global()->activeTime() ) )
            continue;

        // Translate to appropriate z value
        double z = viewSettings.zFromT(t);
        glPushMatrix();
        glScaled(1, -1, 1);
        glTranslated(0,0,z);

        glDisable(GL_LIGHTING);

        glPushMatrix();
        double eps = 1.0e-2;
        foreach(KeyCell * c, list)
        {
            if(viewSettings.drawFramesAsTopology())
                c->drawTopology(t, view2DSettings);
            else
                c->draw(t, view2DSettings);

            glTranslated(0,0,eps);
        }
        glPopMatrix();
    }
}

void VAC::draw3D(View3DSettings & viewSettings)
{
    // Draw grid
    if(viewSettings.drawGrid())
        drawInbetweenGrid(viewSettings);

    // Get appropriate 2D settings
    ViewSettings view2DSettings = global()->activeView()->viewSettings();
    view2DSettings.setScreenRelative(false);
    view2DSettings.setVertexTopologySize(viewSettings.vertexTopologySize());
    view2DSettings.setEdgeTopologyWidth(viewSettings.edgeTopologyWidth());
    view2DSettings.setDrawTopologyFaces(viewSettings.drawTopologyFaces());

    // Draw current frame
    //
    // Note: if we do this after doing drawAllFrames3D() instead of before,
    // then we get an interesting effect: the current frame is not obscured by
    // other frames. Even though it shouldn't be the default, there might be
    // use cases where it is useful and could be added as a settings:
    //   [ ] Current frame not obscured by other frames
    glDepthFunc(GL_ALWAYS);
    if(viewSettings.drawCurrentFrame())
        drawOneFrame3D(global()->activeTime(), viewSettings, view2DSettings, viewSettings.drawCurrentFrameAsTopology());
    glDepthFunc(GL_LESS);

    // Draw all frames
    //
    // XXX This should probably not draw the current current frame if
    // drawCurrentFrame() is true, since it will be drawn anyway below
    //
    // XXX glDepthFunc(GL_ALWAYS); should be disabled too for this, but in order
    // to work correctly we first need to order the frames back to front. It
    // would fix the ugly "z-translation-by-epsilon"
    // that is currently done in drawOneFrame3D. It would be cleaner, more
    // robust, and give better result (no z-fighting).
    if(viewSettings.drawAllFrames())
        drawAllFrames3D(viewSettings, view2DSettings);

    // draw key cells
    if(viewSettings.drawKeyCells())
        drawKeyCells3D(viewSettings, view2DSettings);

    // Draw inbetween cells
    if(viewSettings.drawInbetweenCells())
    {
        // Draw inbetween vertices
        InbetweenVertexSet inbetweenVertices = cells();
        foreach(InbetweenVertex * v, inbetweenVertices)
            v->draw3D(viewSettings);

        // Draw inbetween edges
        //glEnable(GL_LIGHTING);

        GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
        GLfloat mat_shininess[] = { 50.0 };
        GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
        glClearColor (0.0, 0.0, 0.0, 0.0);
        glShadeModel (GL_SMOOTH);

        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);

        glEnable(GL_NORMALIZE);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(false);

        //drawSphere(0.05,50,50);

        if(viewSettings.drawAsMesh())
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        //glColor4d(0.5,1.0,0.5,viewSettings.opacity());
        glColor4d(1.0,0.5,0.5,viewSettings.opacity());
        InbetweenEdgeSet inbetweenEdges = cells();
        foreach(InbetweenEdge * e, inbetweenEdges)
            e->draw3D(viewSettings);
        if(viewSettings.drawAsMesh())
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    }
}

void VAC::drawPick3D(View3DSettings & /*viewSettings*/)
{
}

void VAC::drawInbetweenGrid(View3DSettings & viewSettings)
{
    Timeline * timeline = global()->timeline();
    int firstFrame = timeline->firstFrame();
    int lastFrame = timeline->lastFrame();

    // Get scene values
    double xSceneMin = viewSettings.xSceneMin();
    double xSceneMax = viewSettings.xSceneMax();
    double ySceneMin = viewSettings.ySceneMin();
    double ySceneMax = viewSettings.ySceneMax();

    // Convert to OpenGL units
    double xMin = viewSettings.xFromX2D(xSceneMin);
    double xMax = viewSettings.xFromX2D(xSceneMax);
    double yMin = viewSettings.yFromY2D(ySceneMin);
    double yMax = viewSettings.yFromY2D(ySceneMax);

    glDisable(GL_LIGHTING);
    glLineWidth(1.0);
    glColor3f(0.5, 0.5, 0.5);

    for(int i=firstFrame; i<=lastFrame; ++i)
    {
        double z = viewSettings.zFromT(i);

        glBegin(GL_LINE_LOOP);
        glVertex3d(xMin, yMin, z);
        glVertex3d(xMin, yMax, z);
        glVertex3d(xMax, yMax, z);
        glVertex3d(xMax, yMin, z);
        glEnd();
    }
}

void VAC::draw(Time time, ViewSettings & viewSettings)
{
    ViewSettings::DisplayMode displayMode = viewSettings.displayMode();

    // Illustration mode
    if( (displayMode == ViewSettings::ILLUSTRATION))
    {
        // Draw all cells
        for(auto c: zOrdering_)
            c->draw(time, viewSettings);

        // Draw sketched edge
        if(sketchedEdge_)
            drawSketchedEdge(time, viewSettings);
    }

    // Outline only mode
    else if( (displayMode == ViewSettings::OUTLINE) )
    {
        // Draw all cells
        for(auto c: zOrdering_)
            c->drawTopology(time, viewSettings);

        // Draw sketched edge
        if(sketchedEdge_)
            drawTopologySketchedEdge(time, viewSettings);
    }

    // Illustration + Outline mode
    else if( (displayMode == ViewSettings::ILLUSTRATION_OUTLINE) )
    {
        // First pass
        for(auto c: zOrdering_)
            c->draw(time, viewSettings);
        if(sketchedEdge_)
            drawSketchedEdge(time, viewSettings);

        // Second pass
        for(auto c: zOrdering_)
            c->drawTopology(time, viewSettings);
        if(sketchedEdge_)
            drawTopologySketchedEdge(time, viewSettings);
    }

    // Draw to be painted face
    if( (global()->toolMode() == Global::PAINT) &&
            toBePaintedFace_)
    {
        toBePaintedFace_->draw(viewSettings);
    }

    // Draw sculpt cursor
    if(viewSettings.drawCursor()
            && global()->toolMode() == Global::SCULPT
            && sculptedEdge_
            && !(hoveredCell_
                 && (hoveredCell_->toKeyVertex()
                     // || selectedCells_.contains(highlightedCell_)
                     )
                 )
            && global()->hoveredView()
            && global()->hoveredView()->activeTime() == time)
    {
        // set color of cursor
        glColor3d(1,0,0);

        // draw point on instant edge
        int n = 50;
        EdgeSample p = sculptedEdge_->geometry()->sculptVertex();
        glBegin(GL_POLYGON);
        {
            double r = 0.5 * p.width();
            if(displayMode == ViewSettings::ILLUSTRATION_OUTLINE ||
                    displayMode == ViewSettings::OUTLINE )
            {
                r = 5.0  / viewSettings.zoom();
            }
            else if ( r == 0 )
            {
                r = 3.0 / viewSettings.zoom();
            }
            else if (r * viewSettings.zoom() < 1)
            {
                r = 1.0 / viewSettings.zoom();
            }
            for(int i=0; i<n; ++i)
            {
                double theta = 2 * (double) i * 3.14159 / (double) n ;
                glVertex2d(p.x() + r*std::cos(theta),p.y()+ r*std::sin(theta));
            }
        }
        glEnd();

        // draw circle of influence
        glLineWidth(1);
        glBegin(GL_LINE_LOOP);
        {
            double r = global()->sculptRadius();
            for(int i=0; i<n; ++i)
            {
                double theta = 2 * (double) i * 3.14159 / (double) n ;
                glVertex2d(p.x() + r*std::cos(theta),p.y()+ r*std::sin(theta));
            }
        }
        glEnd();
    }

    // Draw pen radius and snap threshold
    if(viewSettings.drawCursor()
            && global()->toolMode() == Global::SKETCH
            && global()->hoveredView()
            && global()->hoveredView()->activeTime() == time)
    {

        // Set color of cursor. We enforce alpha>0.2 to make sure users see something
        QColor color = global()->edgeColor();
        glColor4d(color.redF(),color.greenF(),color.blueF(), std::max(0.2, color.alphaF()));

        // Get position of cursor in scene coordinates
        Eigen::Vector2d p = global()->sceneCursorPos();

        // Draw pen cursor position + radius as disk
        int n = 50;
        glBegin(GL_POLYGON);
        {
            // Note: Unlike for the sculpt radius widget, we always draw the sketch widget with the actual
            //       drawn width even in topology mode, since we want to give feedback to the user to what's
            //       drawn under the hood
            double r = 0.5 * global()->edgeWidth();

            //if(displayMode == ViewSettings::ILLUSTRATION_OUTLINE ||
            //   displayMode == ViewSettings::OUTLINE )
            //{
            //    r = 5.0  / viewSettings.zoom();
            //}
            //else
            if ( r == 0 )
            {
                r = 3.0 / viewSettings.zoom();
            }
            else if (r * viewSettings.zoom() < 1)
            {
                r = 1.0 / viewSettings.zoom();
            }
            for(int i=0; i<n; ++i)
            {
                double theta = 2 * (double) i * 3.14159 / (double) n ;
                glVertex2d(p[0] + r*std::cos(theta),p[1]+ r*std::sin(theta));
            }
        }
        glEnd();

        // draw snap radius
        if(global()->snapMode())
        {
            glLineWidth(1);
            glBegin(GL_LINE_LOOP);
            {
                double r = global()->snapThreshold();
                for(int i=0; i<n; ++i)
                {
                    double theta = 2 * (double) i * 3.14159 / (double) n ;
                    glVertex2d(p[0] + r*std::cos(theta),p[1]+ r*std::sin(theta));
                }
            }
            glEnd();
        }


    }

    // Rectangle of selection
    if(drawRectangleOfSelection_ && viewSettings.isMainDrawing())
    {
        glColor4d(0.5,0.5,0.8,0.2);
        glLineWidth(1);
        glBegin(GL_QUADS);
        {
            glVertex2d(rectangleOfSelectionStartX_,rectangleOfSelectionStartY_);
            glVertex2d(rectangleOfSelectionStartX_,rectangleOfSelectionEndY_);
            glVertex2d(rectangleOfSelectionEndX_,rectangleOfSelectionEndY_);
            glVertex2d(rectangleOfSelectionEndX_,rectangleOfSelectionStartY_);
        }
        glEnd();
        glColor4d(0,0,0,1);
        glLineWidth(1);
        glBegin(GL_LINE_LOOP);
        {
            glVertex2d(rectangleOfSelectionStartX_,rectangleOfSelectionStartY_);
            glVertex2d(rectangleOfSelectionStartX_,rectangleOfSelectionEndY_);
            glVertex2d(rectangleOfSelectionEndX_,rectangleOfSelectionEndY_);
            glVertex2d(rectangleOfSelectionEndX_,rectangleOfSelectionStartY_);
        }
        glEnd();
    }

    // Draw edge orientation
    if(DevSettings::getBool("draw edge orientation"))
    {
        KeyEdgeSet edges = cells();
        foreach(KeyEdge * e, edges)
        {
            if(e->exists(time))
            {
                double l = e->geometry()->length();
                Eigen::Vector2d p = e->geometry()->pos2d(0.5*l);
                Eigen::Vector2d u = e->geometry()->der(0.5*l);
                GLUtils::drawArrow(p,u);
            }
        }
        InbetweenEdgeSet sedges = cells();
        foreach(InbetweenEdge * se, sedges)
        {
            if(se->exists(time))
            {
                QList<EdgeSample> samples = se->getSampling(time);
                LinearSpline ls(samples);
                double l = ls.length();
                Eigen::Vector2d p = ls.pos2d(0.5*l);
                Eigen::Vector2d u = ls.der(0.5*l);
                GLUtils::drawArrow(p,u);
            }
        }
    }
}

void VAC::drawPick(Time time, ViewSettings & viewSettings)
{
    ViewSettings::DisplayMode displayMode = viewSettings.displayMode();

    if( (displayMode == ViewSettings::ILLUSTRATION) )
    {
        // Draw all cells
        for(auto c: zOrdering_)
        {
            c->drawPick(time, viewSettings);
        }
    }

    else if( (displayMode == ViewSettings::OUTLINE) )
    {
        // Draw all cells
        for(auto c: zOrdering_)
        {
            c->drawPickTopology(time, viewSettings);
        }
    }

    else if( (displayMode == ViewSettings::ILLUSTRATION_OUTLINE) )
    {
        // first pass: pick faces normally
        for(auto c: zOrdering_)
        {
            if(c->toFaceCell())
                c->drawPick(time, viewSettings);
        }


        // second pass: pick vertices and edges as outline
        for(auto c: zOrdering_)
        {
            if(!c->toFaceCell())
                c->drawPickTopology(time, viewSettings);
        }
    }
}


void VAC::emitSelectionChanged_()
{
    if(signalCounter_ == 0)
    {
        emit selectionChanged();
        informTimelineOfSelection();
    }
    else
    {
        shouldEmitSelectionChanged_ = true;
    }
}

void VAC::beginAggregateSignals_()
{
    if(signalCounter_ == 0)
    {
        shouldEmitSelectionChanged_ = false;
    }

    signalCounter_++;
}

void VAC::endAggregateSignals_()
{
    signalCounter_--;

    if(signalCounter_ == 0)
    {
        if(shouldEmitSelectionChanged_)
            emit selectionChanged();
    }
}



// ----------------- Selecting and Highlighting ----------------

// Should NOT emit changed(). View does it if necessary


void VAC::setHoveredObject(Time /*time*/, int id)
{
    setHoveredCell(getCell(id));
}

void VAC::setNoHoveredObject()
{
    setNoHoveredCell();
}

void VAC::select(Time /*time*/, int id)
{
    addToSelection(getCell(id),false);
}

void VAC::deselect(Time /*time*/, int id)
{
    removeFromSelection(getCell(id),false);
}

void VAC::toggle(Time /*time*/, int id)
{
    toggleSelection(getCell(id),false);
}

void VAC::deselectAll(Time time)
{
    CellSet cellsToDeselect;
    foreach(Cell * cell, selectedCells())
        if(cell->exists(time))
            cellsToDeselect << cell;
    removeFromSelection(cellsToDeselect,false);
}


void VAC::deselectAll()
{
    if(numSelectedCells() != 0)
        setSelectedCells(CellSet(),false);
}

void VAC::invertSelection()
{
    QSet<Cell *> newSelectedCells = cells();
    newSelectedCells.subtract(selectedCells());
    setSelectedCells(newSelectedCells);
}

Cell * VAC::hoveredCell() const
{
    return hoveredCell_;
}

CellSet VAC::selectedCells() const
{
    return selectedCells_;
}

int VAC::numSelectedCells() const
{
    return selectedCells_.size();
}

// ----------------------  Save & Load -------------------------

void VAC::write(XmlStreamWriter & xml)
{
    for(Cell * cell: zOrdering_)
        cell->write(xml);
}

void VAC::clear()
{
    deleteAllCells();
    initNonCopyable();
    initCopyable();
}

void VAC::read(XmlStreamReader & xml)
{
    clear();

    while (xml.readNextStartElement())
    {
        Cell * cell = 0;

        if(xml.name() == "vertex")
            cell = new KeyVertex(this, xml);
        else if(xml.name() == "edge")
            cell = new KeyEdge(this, xml);
        else if(xml.name() == "face")
            cell = new KeyFace(this, xml);
        else if(xml.name() == "inbetweenvertex")
            cell = new InbetweenVertex(this, xml);
        else if(xml.name() == "inbetweenedge")
            cell = new InbetweenEdge(this, xml);
        else if(xml.name() == "inbetweenface")
            cell = new InbetweenFace(this, xml);

        xml.skipCurrentElement(); // XXX this should be in "Cell(this, xml)"

        if(cell)
        {
            int id = cell->id();
            if(id > maxID_)
                maxID_ = id;
            cells_.insert(id, cell);
            zOrdering_.insertLast(cell);
        }
    }

    read2ndPass_();
}

void VAC::read2ndPass_()
{
    // Convert temp IDs (int) to pointers (Cell*)
    foreach(Cell * cell, cells_)
        cell->read2ndPass();

    // Create star from boundary
    foreach(Cell * cell, cells_)
    {
        CellSet spatialBoundary = cell->spatialBoundary();
        foreach(Cell * bcell, spatialBoundary)
            cell->addMeToSpatialStarOf_(bcell);

        CellSet temporalBoundaryBefore = cell->beforeCells();
        foreach(Cell * bcell, temporalBoundaryBefore)
            cell->addMeToTemporalStarAfterOf_(bcell);

        CellSet temporalBoundaryAfter = cell->afterCells();
        foreach(Cell * bcell, temporalBoundaryAfter)
            cell->addMeToTemporalStarBeforeOf_(bcell);
    }

    // Clean geometry
    foreach(Cell * cell, cells_)
    {
        KeyEdge * kedge = cell->toKeyEdge();
        if(kedge)
        {
            kedge->correctGeometry();
        }
    }
}

void VAC::save_(QTextStream & out)
{
    // list of objects
    out << Save::newField("Cells");
    out << "\n" << Save::indent() << "[";
    Save::incrIndent();
    for(Cell * obj: zOrdering_)
    {
        out << Save::openCurlyBrackets();
        obj->save(out);
        out << Save::closeCurlyBrackets();
    }
    Save::decrIndent();
    out << "\n" << Save::indent() << "]";
}

void VAC::exportSVG_(Time t, QTextStream & out)
{
    // list of objects
    for(Cell * c: zOrdering_)
    {
        if(c->exists(t))
            c->exportSVG(t, out);
    }
}

VAC::VAC(QTextStream & in) :
    SceneObject()
{
    clear();

    Field field;

    // list of objects
    // -- 1st pass: construct temp objects storing IDs instead of pointers
    in >> field; // Cells
    Read::skipBracket(in); // [
    while(Read::string(in) == "{")
    {
        Cell * cell = Cell::read1stPass(this, in);
        int id = cell->id();
        if(id > maxID_)
            maxID_ = id;
        cells_.insert(id, cell);
        zOrdering_.insertLast(cell);
        Read::skipBracket(in); // }
    }
    // last read string == ]

    read2ndPass_();
}



// ------------------- Accessing elements ----------------------

Cell * VAC::getCell(int id)
{
    if(cells_.contains(id))
        return cells_[id];
    else
        return 0;
}

KeyVertex * VAC::getKeyVertex(int id)
{
    Cell * object = getCell(id);
    if(object)
        return object->toKeyVertex();
    else
        return 0;
}

KeyEdge * VAC::getKeyEdge(int id)
{
    Cell * object = getCell(id);
    if(object)
        return object->toKeyEdge();
    else
        return 0;
}

KeyFace * VAC::getKeyFace(int /*id*/)
{
    return 0;
}

InbetweenVertex * VAC::getInbetweenVertex(int /*id*/)
{
    return 0;
}

InbetweenEdge * VAC::getInbetweenEdge(int /*id*/)
{
    return 0;
}

InbetweenFace * VAC::getInbetweenFace(int /*id*/)
{
    return 0;
}

CellSet VAC::cells()
{
    CellSet res;
    foreach(Cell * obj, cells_)
        res << obj;
    return res;
}

VertexCellList VAC::vertices()
{
    VertexCellList res;
    foreach(Cell * o, cells_)
    {
        VertexCell *node = o->toVertexCell();
        if(node)
            res << node;
    }
    return res;
}
KeyVertexList VAC::instantVertices()
{
    KeyVertexList res;
    foreach(Cell * o, cells_)
    {
        KeyVertex *node = o->toKeyVertex();
        if(node)
            res << node;
    }
    return res;
}

EdgeCellList VAC::edges()
{
    EdgeCellList res;
    foreach(Cell * o, cells_)
    {
        EdgeCell *edge = o->toEdgeCell();
        if(edge)
            res << edge;
    }
    return res;
}

EdgeCellList VAC::edges(Time time)
{
    EdgeCellList res;
    foreach(Cell * o, cells_)
    {
        EdgeCell *edge = o->toEdgeCell();
        if(edge && edge->exists(time))
            res << edge;
    }
    return res;
}

FaceCellList VAC::faces()
{
    FaceCellList res;
    foreach(Cell * o, cells_)
    {
        FaceCell *face = o->toFaceCell();
        if(face)
            res << face;
    }
    return res;
}


KeyEdgeList VAC::instantEdges()
{
    KeyEdgeList res;
    foreach(Cell * o, cells_)
    {
        KeyEdge * iedge = o->toKeyEdge();
        if(iedge)
            res << iedge;
    }
    return res;
}

KeyVertexList VAC::instantVertices(Time time)
{
    KeyVertexList res;
    foreach(Cell * o, cells_)
    {
        KeyVertex *node = o->toKeyVertex();
        if(node && node->exists(time))
            res << node;
    }
    return res;
}

KeyEdgeList VAC::instantEdges(Time time)
{
    KeyEdgeList res;
    foreach(Cell * o, cells_)
    {
        KeyEdge * iedge = o->toKeyEdge();
        if(iedge && iedge->exists(time))
            res << iedge;
    }
    return res;
}

// ----------------------- Managing IDs ------------------------

int VAC::getAvailableID()
{
    return ++maxID_; // increments maxId_ then returns it
}

void VAC::insertCell_(Cell * cell)
{
    int id = getAvailableID();
    cell->id_ = id;
    cell->vac_ = this;
    cells_.insert(id, cell);
    zOrdering_.insertCell(cell);
}

void VAC::insertCellLast_(Cell * cell)
{
    int id = getAvailableID();
    cell->id_ = id;
    cell->vac_ = this;
    cells_.insert(id, cell);
    zOrdering_.insertLast(cell);
}

void VAC::removeCell_(Cell * cell)
{
    if(cell)
    {
        cells_.remove(cell->id());
        zOrdering_.removeCell(cell);
        removeFromSelection(cell,false);
        if(cell->isSelected())
        {
            removeFromSelection(cell,false);
        }
        if(cell->isHovered())
        {
            cell->setHovered(false);
            hoveredCell_ = 0;
        }
        if(cell == sculptedEdge_)
            sculptedEdge_ = 0;

        if(cell == hoveredFaceOnMousePress_)
            hoveredFaceOnMousePress_ = 0;
        if(cell == hoveredFaceOnMouseRelease_)
            hoveredFaceOnMouseRelease_= 0;
        hoveredFacesOnMouseMove_.remove(cell->toKeyFace());
        facesToConsiderForCutting_.remove(cell->toKeyFace());
    }
}


void VAC::smartDelete_(const CellSet & cellsToDelete)
{
    // Note: we know that deleting or simplifying a cell of dimension N
    // leave untouched any cell of dimension <= N.

    // For instance:
    //  - Deleting a face leaves untouched other faces, edges and vertices
    //  - Simplifying a face does nothing
    //  - Deleting an edge also deletes incident faces, but leaves untouched
    //    other edges and vertices
    //  - Simplifying an edge may merge incident faces (hence, delete them
    //    and create a new one) but leaves untouched other edges and vertices
    //  - Deleting a vertex may delete faces and edges, but leaves untouched
    //    other vertices
    //  - Simplifying a vertex may merge incident faces/edges, but leaves
    //    untouched other vertices

    // Hence, by first considering faces, then edges, then vertices, we don't
    // have to recompute sets
    KeyFaceSet facesToDelete = cellsToDelete;
    KeyEdgeSet edgesToDelete= cellsToDelete;
    KeyVertexSet verticesToDelete = cellsToDelete;

    foreach(KeyFace * iface, facesToDelete)
        smartDeleteCell(iface);

    foreach(KeyEdge * iedge, edgesToDelete)
        smartDeleteCell(iedge);

    foreach(KeyVertex * ivertex, verticesToDelete)
        smartDeleteCell(ivertex);
}

void VAC::smartDelete()
{
    if(numSelectedCells() == 0)
        return;

    // Prepare automatic cleaning of vertices
    //CellSet cellsToDelete = selectedCells();
    //CellSet closureOfCellsToDelete = Algorithms::closure(cellsToDelete);
    //KeyVertexSet verticesToDelete = cellsToDelete;
    //KeyVertexSet verticesToConsiderForCleaning = closureOfCellsToDelete;
    //verticesToConsiderForCleaning.subtract(verticesToDelete);
    // => doesn't work, because "closure" is not enough
    //       v1 ---------- v2
    //  then deleting v1 should also delete v2 since e is deleted as a side effect

    smartDelete_(selectedCells());

    // Automatic cleaning of vertices
    // naive method for now, not efficient but works
    if(global()->deleteIsolatedVertices())
    {
        foreach(KeyVertex * keyVertex, instantVertices())
        {
            if(keyVertex->star().isEmpty())
                deleteCell(keyVertex);
        }
    }

    emit needUpdatePicking();
    emit changed();
    emit checkpoint();
}

void VAC::deleteSelectedCells()
{
    if(numSelectedCells() == 0)
        return;

    while(numSelectedCells() != 0)
    {
        Cell * obj = *selectedCells().begin();
        deleteCell(obj);
    }

    // Automatic cleaning of vertices
    // naive method for now, not efficient but works
    if(global()->deleteIsolatedVertices())
    {
        foreach(KeyVertex * keyVertex, instantVertices())
        {
            if(keyVertex->star().isEmpty())
                deleteCell(keyVertex);
        }
    }

    emit needUpdatePicking();
    emit changed();
    emit checkpoint();
}

void VAC::deleteCells(const QSet<int>  & cellIds)
{
    foreach(int id, cellIds)
    {
        // Get cell corresponding to ID
        Cell * cell = getCell(id);

        // Note: cell might be NULL, as it might have been recursively deleted
        //       that's why this method has to be implemented with IDs, as pointers
        //       can become invalid if implemented directly with QSet<Cell*>
        if(cell)
            deleteCell(cell);
    }
}

void VAC::deleteCells(const CellSet & cells)
{
    QSet<int> cellIds;
    foreach(Cell * c, cells)
        cellIds << c->id();
    deleteCells(cellIds);
}

void VAC::deleteCell(Cell * cell)
{
    // Recusrively delete star cells first (complex remains valid upon return)
    cell->destroyStar();

    // Inform observers of the upcoming deletion
    foreach(CellObserver * observer, cell->observers_)
        observer->observedCellDeleted(cell);

    // Remove the cell from the star of its boundary
    cell->informBoundaryImGettingDestroyed();

    // Remove the cell from the various cell sets, e.g.:
    //  - id to pointer map
    //  - depth ordering
    //  - selection
    //  - ...
    removeCell_(cell);

    // Finally, now that no pointers point to the cell, release memory
    delete cell;
}

// Smart deletion and simplification
bool VAC::atomicSimplifyAtCell(Cell * cell)
{
    KeyVertex * ivertex = cell->toKeyVertex();
    KeyEdge * iedge = cell->toKeyEdge();
    if(ivertex)
    {
        return uncut_(ivertex);
    }
    else if(iedge)
    {
        return uncut_(iedge);
    }
    else
    {
        return false;
    }
}

void VAC::smartDeleteCell(Cell * cell)
{
    bool success = atomicSimplifyAtCell(cell);

    if(success)
    {
        // Yay!
    }
    else
    {
        // Try to smart simplify direct star first
        // for now, since I do not have a directStar()
        // abstract method, I do things more manually.
        CellSet star = cell->star();

        KeyEdgeSet starEdges = star;
        if(!starEdges.isEmpty())
        {
            // Great, this means starEdges is the direct star of cell
            // Atomic simplify all of them
            foreach(KeyEdge * iedge, starEdges)
            {
                atomicSimplifyAtCell(iedge);
            }

            // Ok, so maybe now it is possible to simplify at this cell
            bool newTrySuccess = atomicSimplifyAtCell(cell);
            if(newTrySuccess)
            {
                // Yay!
            }
            else
            {
                // There's definitely nothing we can't do about this cell
                // the 1-skeleton of its extendedStar is non-manifold, and
                // the 2-skeleton of its extendedStar is non-manifold either.
                deleteCell(cell);
            }
        }
        else
        {
            // This means starFaces is the direct star of cell
            // Great: since we do not consider volumes, there is
            // no way faces can be simplified
            // Hence, we know for sure
            // Atomic simplify all of them atomicSimplifyAtCell(cell) == false.
            //
            // So there's nothing we can do about this cell
            deleteCell(cell);
        }
    }
}

void VAC::deleteAllCells()
{
    while(!cells_.isEmpty())
    {
        Cell * obj = *cells_.begin();
        deleteCell(obj);
    }
    maxID_ = -1;
}

KeyVertex* VAC::newKeyVertex(Time time, const Eigen::Vector2d & pos)
{
    KeyVertex * node = new KeyVertex(this, time, pos);
    insertCell_(node);
    return node;
}

KeyEdge * VAC::newKeyEdge(Time time,
                          KeyVertex * left,
                          KeyVertex * right,
                          EdgeGeometry * geometry, double width)
{
    if(geometry == 0)
    {
        // Create straight invisible edge
        EdgeSample startSample(left->pos()[0], left->pos()[1], width);
        EdgeSample endSample(right->pos()[0], right->pos()[1], width);
        SculptCurve::Curve<EdgeSample> curve(startSample, endSample);
        geometry = new LinearSpline(curve);
    }

    KeyEdge * edge = new KeyEdge(this, time, left, right, geometry);
    insertCell_(edge);
    return edge;
}

KeyEdge * VAC::newKeyEdge(Time time,
                          EdgeGeometry * geometry)
{
    if(geometry == 0)
        geometry = new EdgeGeometry();

    geometry->makeLoop();
    KeyEdge * edge = new KeyEdge(this, time, geometry);
    insertCell_(edge);
    return edge;
}

InbetweenVertex * VAC::newInbetweenVertex(
        KeyVertex * before,
        KeyVertex * after)
{
    InbetweenVertex * svertex = new InbetweenVertex(this, before, after);
    insertCell_(svertex);
    return svertex;
}

InbetweenEdge * VAC::newInbetweenEdge(
        const Path & beforePath,
        const Path & afterPath,
        const AnimatedVertex & startAnimatedVertex,
        const AnimatedVertex & endAnimatedVertex)
{
    InbetweenEdge * sedge = new InbetweenEdge(this, beforePath, afterPath, startAnimatedVertex, endAnimatedVertex);
    insertCell_(sedge);
    return sedge;
}
InbetweenEdge * VAC::newInbetweenEdge(
        const Cycle & beforeCycle,
        const Cycle & afterCycle)
{
    InbetweenEdge * sedge = new InbetweenEdge(this, beforeCycle, afterCycle);
    insertCell_(sedge);
    return sedge;
}

KeyFace * VAC::newKeyFace(const Time & t)
{
    KeyFace * face = new KeyFace(this, t);
    insertCell_(face);
    return face;
}

KeyFace * VAC::newKeyFace(const Cycle & cycle)
{
    KeyFace * face = new KeyFace(this, cycle);
    insertCell_(face);
    return face;
}

KeyFace * VAC::newKeyFace(const QList<Cycle> & cycles)
{
    KeyFace * face = new KeyFace(this, cycles);
    insertCell_(face);
    return face;
}
InbetweenFace * VAC::newInbetweenFace(const QList<AnimatedCycle> & cycles,
                                      const QSet<KeyFace*> & beforeFaces,
                                      const QSet<KeyFace*> & afterFaces)
{
    InbetweenFace * sface = new InbetweenFace(this, cycles, beforeFaces, afterFaces);
    insertCell_(sface);
    return sface;
}

// ------------- User action: rectangle of selection -------------

void VAC::beginRectangleOfSelection(double x, double y, Time time)
{
    timeInteractivity_ = time;
    rectangleOfSelectionStartX_ = x;
    rectangleOfSelectionStartY_ = y;
    rectangleOfSelectionEndX_ = x;
    rectangleOfSelectionEndY_ = y;
    drawRectangleOfSelection_ = true;
    rectangleOfSelectionSelectedBefore_ = selectedCells();
}

void VAC::continueRectangleOfSelection(double x, double y)
{
    // Set raw data
    rectangleOfSelectionEndX_ = x;
    rectangleOfSelectionEndY_ = y;

    // Compute clean positive rectangle
    double x0 = rectangleOfSelectionStartX_;
    double x1 = rectangleOfSelectionEndX_;
    double y0 = rectangleOfSelectionStartY_;
    double y1 = rectangleOfSelectionEndY_;
    if(x1 < x0) std::swap(x0,x1);
    if(y1 < y0) std::swap(y0,y1);

    // Compute cells in rectangle of selection
    // Note: This should be factorized: intersectsRectangle() should be
    // a virtual method of Cell
    cellsInRectangleOfSelection_.clear();
    KeyVertexSet vertices = cells();
    KeyEdgeSet edges = cells();
    KeyFaceSet faces = cells();
    InbetweenVertexSet ivertices = cells();
    InbetweenEdgeSet iedges = cells();
    InbetweenFaceSet ifaces = cells();
    foreach(KeyVertex * v, vertices)
    {
        if(v->isPickable(timeInteractivity_))
        {
            Eigen::Vector2d p = v->pos();
            if(x0 <= p[0] && p[0] <= x1
                    && y0 <= p[1] && p[1] <= y1)
            {
                cellsInRectangleOfSelection_ << v;
            }
        }
    }
    foreach(KeyEdge * e, edges)
    {
        if(e->isPickable(timeInteractivity_))
        {
            if(e->intersectsRectangle(timeInteractivity_,x0,x1,y0,y1))
                cellsInRectangleOfSelection_ << e;
        }
    }
    foreach(KeyFace * f, faces)
    {
        if(f->isPickable(timeInteractivity_))
        {
            if(f->intersectsRectangle(timeInteractivity_,x0,x1,y0,y1))
                cellsInRectangleOfSelection_ << f;
        }
    }
    foreach(InbetweenVertex * v, ivertices)
    {
        if(v->isPickable(timeInteractivity_))
        {
            Eigen::Vector2d p = v->pos(timeInteractivity_);
            if(x0 <= p[0] && p[0] <= x1
                    && y0 <= p[1] && p[1] <= y1)
            {
                cellsInRectangleOfSelection_ << v;
            }
        }
    }
    foreach(InbetweenEdge * e, iedges)
    {
        if(e->isPickable(timeInteractivity_))
        {
            if(e->intersectsRectangle(timeInteractivity_,x0,x1,y0,y1))
                cellsInRectangleOfSelection_ << e;
        }
    }
    foreach(InbetweenFace * f, ifaces)
    {
        if(f->isPickable(timeInteractivity_))
        {
            if(f->intersectsRectangle(timeInteractivity_,x0,x1,y0,y1))
                cellsInRectangleOfSelection_ << f;
        }
    }

    setSelectedCellsFromRectangleOfSelection();
}

void VAC::setSelectedCellsFromRectangleOfSelection()
{
    // Get keyboard modifiers to know what to do
    Qt::KeyboardModifiers modifiers = QGuiApplication::keyboardModifiers();
    setSelectedCellsFromRectangleOfSelection(modifiers);
}

void VAC::setSelectedCellsFromRectangleOfSelection(Qt::KeyboardModifiers modifiers)
{
    if(modifiers == Qt::NoModifier)
    {
        // Set selection
        setSelectedCells(cellsInRectangleOfSelection_, false);
    }
    else if(modifiers & Qt::ShiftModifier)
    {
        if(modifiers & Qt::AltModifier)
        {
            // Intersect selection
            CellSet newSelectedSet = rectangleOfSelectionSelectedBefore_;
            newSelectedSet.intersect(cellsInRectangleOfSelection_);
            setSelectedCells(newSelectedSet, false);
        }
        else
        {
            // Add to selection
            CellSet newSelectedSet = rectangleOfSelectionSelectedBefore_;
            newSelectedSet.unite(cellsInRectangleOfSelection_);
            setSelectedCells(newSelectedSet, false);
        }
    }
    else if(modifiers & Qt::AltModifier)
    {
        // Remove from selection
        CellSet newSelectedSet = rectangleOfSelectionSelectedBefore_;
        newSelectedSet.subtract(cellsInRectangleOfSelection_);
        setSelectedCells(newSelectedSet, false);
    }
}

void VAC::endRectangleOfSelection()
{
    drawRectangleOfSelection_ = false;
}

// ------------- User action: drawing a new stroke -------------

void VAC::beginSketchEdge(double x, double y, double w, Time time)
{
    timeInteractivity_ = time;
    sketchedEdge_ = new LinearSpline(ds_);
    sketchedEdge_->beginSketch(EdgeSample(x,y,w));
    hoveredFaceOnMousePress_ = 0;
    hoveredFaceOnMouseRelease_ = 0;
    hoveredFacesOnMouseMove_.clear();
    if(hoveredCell_)
    {
        InbetweenFace * sface = hoveredCell_->toInbetweenFace();
        if(sface && global()->planarMapMode())
            hoveredCell_ = keyframe_(sface, timeInteractivity_);
        hoveredFaceOnMousePress_ = hoveredCell_->toKeyFace();
    }
}

void VAC::continueSketchEdge(double x, double y, double w)
{
    if(sketchedEdge_)
    {
        sketchedEdge_->continueSketch(EdgeSample(x,y,w));
        if(hoveredCell_)
        {
            InbetweenFace * sface = hoveredCell_->toInbetweenFace();
            if(sface && global()->planarMapMode())
                hoveredCell_ = keyframe_(sface, timeInteractivity_);

            KeyFace * hoveredFace = hoveredCell_->toKeyFace();
            if(hoveredFace)
                hoveredFacesOnMouseMove_.insert(hoveredFace);
        }


    }
}

void VAC::endSketchEdge()
{
    if(sketchedEdge_)
    {
        InbetweenFace * sface = hoveredCell_->toInbetweenFace();
        if(sface && global()->planarMapMode())
            hoveredCell_ = keyframe_(sface, timeInteractivity_);

        if(hoveredCell_)
            hoveredFaceOnMouseRelease_ = hoveredCell_->toKeyFace();


        sketchedEdge_->endSketch();

        sketchedEdge_->resample(); // not sure if necessary

        facesToConsiderForCutting_ = hoveredFacesOnMouseMove_;
        if(hoveredFaceOnMousePress_)
            facesToConsiderForCutting_.insert(hoveredFaceOnMousePress_);
        if(hoveredFaceOnMouseRelease_)
            facesToConsiderForCutting_.insert(hoveredFaceOnMouseRelease_);
        insertSketchedEdgeInVAC();

        delete sketchedEdge_;
        sketchedEdge_ = 0;


        //emit changed();
        emit checkpoint();
    }
}

void VAC::beginCutFace(double x, double y, double w, KeyVertex * startVertex)
{
    cut_startVertex_ = startVertex;

    if(cut_startVertex_)
    {
        timeInteractivity_ = cut_startVertex_->time();
        const bool invisibleCut = true;
        if(invisibleCut)
            w = 3.0;

        sketchedEdge_ = new LinearSpline(ds_);
        sketchedEdge_->beginSketch(EdgeSample(x,y,w));

        //emit changed();
    }

}

void VAC::continueCutFace(double x, double y, double w)
{
    if(sketchedEdge_)
    {
        const bool invisibleCut = true;
        if(invisibleCut)
            w = 3.0;

        sketchedEdge_->continueSketch(EdgeSample(x,y,w));
        //emit changed();
    }
}

void VAC::endCutFace(KeyVertex * endVertex)
{
    if(sketchedEdge_)
    {
        bool hasBeenCut = false;

        sketchedEdge_->endSketch();
        sketchedEdge_->resample(); // not sure if necessary

        if(endVertex)
        {
            // convenient alias
            KeyVertex * startVertex = cut_startVertex_;

            // find a face to cut
            KeyFaceSet startFaces = startVertex->spatialStar();
            KeyFaceSet endFaces = endVertex->spatialStar();
            KeyFaceSet faces = startFaces;
            faces.intersect(endFaces);

            if(!faces.isEmpty())
            {
                // For now, just use the first face
                KeyFace * face = *faces.begin();

                // Create the new edge
                EdgeGeometry * geometry = new LinearSpline(sketchedEdge_->curve());
                KeyEdge * iedge = newKeyEdge(timeInteractivity_, startVertex, endVertex, geometry);
                const bool invisibleCut = true;
                if(invisibleCut)
                    iedge->setWidth(0);

                // Cut the face by the new edge
                hasBeenCut = cutFace_(face,iedge);

                if(!hasBeenCut)
                    deleteCell(iedge);
            }

        }

        delete sketchedEdge_;
        sketchedEdge_ = 0;

        if(hasBeenCut)
        {
            //emit changed();
            emit checkpoint();
        }
    }
}

bool VAC::cutFace_(KeyFace * face, KeyEdge * edge, CutFaceFeedback * feedback)
{
    // assumes edge is not a loop
    // assumes edge->start() and edge->end() belong to face boundary

    // Get involved vertices
    KeyVertex * startVertex = edge->startVertex();
    KeyVertex * endVertex = edge->endVertex();

    // Find a suitable use for vStart and vEnd.
    // If vStart has several suitable uses, it chooses the last one
    // If vEnd has several suitable uses, it chooses the last one
    // Better heuristics must use geometry to capture user's intent
    int iStart = -1;
    int iEnd = -1;
    int jStart = -1;
    int jEnd = -1;
    for(int i=0; i<face->cycles_.size(); ++i)
    {
        // convenient alias
        Cycle & cycle = face->cycles_[i];

        switch(cycle.type())
        {
        case Cycle::SingleVertex:
            if(cycle.singleVertex() == startVertex)
            {
                iStart = i;
                jStart = 0;
            }
            if(cycle.singleVertex() == endVertex)
            {
                iEnd = i;
                jEnd = 0;
            }
            break;

        case Cycle::ClosedHalfedge:
            // nothing to do, just ignore it
            break;

        case Cycle::OpenHalfedgeList:
            for(int j=0; j<cycle.size(); ++j)
            {
                KeyHalfedge he = cycle.halfedges_[j];
                KeyVertex * v = he.startVertex();
                if(v == startVertex)
                {
                    iStart = i;
                    jStart = j;
                }
                if(v == endVertex)
                {
                    iEnd = i;
                    jEnd = j;
                }
            }
            break;

        case Cycle::Invalid:
            break;
        }
    }

    // Makes sure they have been found
    if(iStart == -1 || iEnd == -1)
    {
        qDebug() << "CutFace: abort, endVertices of edge not used in face";
        return false;
    }

    // Case where they belong to the same cycle (can be a Steiner cycle)
    if(iStart == iEnd)
    {
        // Convenient alias to the cycle to cut
        int i = iStart; // (= iEnd too)
        Cycle & oldCycle = face->cycles_[i];
        int n = oldCycle.size();

        // Cut the cycle in two:
        //   * Cycle 1 goes from end to start
        //   * Cycle 2 goes from start to end
        Cycle newCycle1;
        Cycle newCycle2;

        // Special case where the uses chosen for vStart and vEnd are equal
        if(jEnd == jStart)
        {
            // Special case in special case where it is a Steiner vertex
            if(oldCycle.type() == Cycle::SingleVertex)
            {
                // The two cycles to use are:
                //   Cycle 1 = [ ]
                //   Cycle 2 = [ ]
                //
                // Yet invalid, but will become valid by adding (e,true) or/and (e,false)
            }
            else
            {
                // The two cycles to use are:
                //   Cycle 1 = [ oldCycle ]
                //   Cycle 2 = [ ]
                //
                // Cycle 2 Yet invalid, but will become valid by adding (e,true) or/and (e,false)
                newCycle1.halfedges_ << oldCycle[jEnd];  // These two lines (first iteration of the loop) are necessary
                jEnd = (jEnd+1) % n;                     // otherwise since jEnd == jStart, the loop would stop instantly
                for(int j = jEnd; j != jStart; j = (j+1) % n)
                    newCycle1.halfedges_ << oldCycle[j];
            }
        }
        // Normal case, where the two uses for vStart and vEnd are different
        else
        {
            // Old Cycle = [ h0 ... h(jStart-1) | h(jStart) ... h(jEnd-1) | h(jEnd) ... h(n-1) ]
            // Cycle 1   = [ h(jEnd) ... h(n-1) | h0 ... h(jStart-1) ]
            // Cycle 2   = [ h(jStart) ... h(jEnd-1) ]

            for(int j = jEnd; j != jStart; j = (j+1) % n )
                newCycle1.halfedges_ << oldCycle[j];

            for(int j = jStart; j != jEnd; j = (j+1) % n )
                newCycle2.halfedges_ << oldCycle[j];
        }

        // Heuristic to decide between doing a Mobius cut or a normal cut
        bool moebiusCut = DevSettings::getBool("mobius cut");
        if(moebiusCut)
        {
            // New Cycle = [ Cycle1 | (e,true) | Cycle2.opposite() | (e,true) ]
            Cycle newCycle;

            // Append Cycle1
            for(int j = 0; j < newCycle1.size(); ++j)
                newCycle.halfedges_ << newCycle1[j];

            // Append (e,true)
            newCycle.halfedges_ << KeyHalfedge(edge, true);

            // Append Cycle2.opposite()
            for(int j = newCycle2.size()-1; j >= 0 ; --j)
                newCycle.halfedges_ << newCycle2[j].opposite();

            // Append (e,true)
            newCycle.halfedges_ << KeyHalfedge(edge, true);

            // Compute new cycles of f
            face->cycles_[i] = newCycle;
            face->addMeToSpatialStarOf_(edge);
            face->geometryChanged_();
        }
        else
        {
            // Cycle 1 <- [ Cycle1 | (e,true) ]
            newCycle1.halfedges_ << KeyHalfedge(edge, true);

            // Cycle 2 <- [ Cycle2 | (e,false) ]
            newCycle2.halfedges_ << KeyHalfedge(edge, false);

            // Create the new faces
            KeyFace * f1 = newKeyFace(newCycle1);
            KeyFace * f2 = newKeyFace(newCycle2);
            if(feedback)
            {
                feedback->newFaces.insert(f1);
                feedback->newFaces.insert(f2);
            }
            // Transfer other cycles to either f1 or f2 using a heuristic
            PreviewKeyFace f1Preview(newCycle1);
            for(int k=0; k<face->cycles_.size(); ++k)
            {
                if(k != i)
                {
                    if(isCycleContainedInFace(face->cycles_[k],f1Preview))
                        f1->addCycle(face->cycles_[k]);
                    else
                        f2->addCycle(face->cycles_[k]);
                }
            }

            // Set their color to be the same of the cut face
            QColor color = face->color();
            f1->setColor(color);
            f2->setColor(color);

            //Set depth-ordering of new faces to be just below the old face
            zOrdering_.moveBelow(f1,face);
            zOrdering_.moveBelow(f2,face);

            // Update star
            InbetweenFaceSet sfacesbefore = face->temporalStarBefore();
            foreach(InbetweenFace * sface, sfacesbefore)
            {
                sface->removeAfterFace(face);
                sface->addAfterFace(f1);
                sface->addAfterFace(f2);
            }
            InbetweenFaceSet sfacesafter = face->temporalStarAfter();
            foreach(InbetweenFace * sface, sfacesafter)
            {
                sface->removeBeforeFace(face);
                sface->addBeforeFace(f1);
                sface->addBeforeFace(f2);
            }

            // Delete old face
            deleteCell(face);
            if(feedback)
            {
                feedback->deletedFaces.insert(face);
            }
        }
    }
    // case where they belong to different cycles
    else
    {
        // Compute the new cycles
        int nStart = face->cycles_[iStart].size();
        int nEnd = face->cycles_[iEnd].size();
        QList<Cycle> newCycles;

        // Joined cycle =     [ | (e,true) |      oldCycleEnd      | (e,false) | oldCycleStart | ]
        //                    vStart      vEnd                    vEnd       vStart          vStart
        //                 or [ | (e,true) | oldCycleEnd.reverse() | (e,false) | oldCycleStart | ]
        Cycle joinedCycle;

        // Append (e,true)
        joinedCycle.halfedges_ << KeyHalfedge(edge, true);

        // Heuristic to reverse or not
        bool reverseOldCycleEnd = true;
        int turningNumberStart = face->cycles_[iStart].turningNumber();
        int turningNumberEnd = face->cycles_[iEnd].turningNumber();
        qDebug() << turningNumberStart << turningNumberEnd;
        if( turningNumberStart * turningNumberEnd < 0)
            reverseOldCycleEnd = false;

        // Append oldCycleEnd or oldCycleEnd.reverse()
        if(reverseOldCycleEnd)
        {
            for(int j = jEnd-1; j >= 0; --j)
                joinedCycle.halfedges_ << face->cycles_[iEnd][j].opposite();
            for(int j = nEnd-1; j >= jEnd; --j)
                joinedCycle.halfedges_ << face->cycles_[iEnd][j].opposite();
        }
        else
        {
            for(int j = jEnd; j != nEnd; ++j)
                joinedCycle.halfedges_ << face->cycles_[iEnd][j];
            for(int j = 0; j != jEnd; ++j)
                joinedCycle.halfedges_ << face->cycles_[iEnd][j];
        }

        // Append (e,false)
        joinedCycle.halfedges_ << KeyHalfedge(edge, false);

        // Append oldCycleStart
        for(int j = jStart; j != nStart; ++j)
            joinedCycle.halfedges_ << face->cycles_[iStart][j];
        for(int j = 0; j != jStart; ++j)
            joinedCycle.halfedges_ << face->cycles_[iStart][j];

        // Add joined cycle to face cycles
        newCycles << joinedCycle;

        // Add other cycles to face cycles
        for(int i=0; i<face->cycles_.size(); ++i)
        {
            if(i != iStart && i != iEnd)
                newCycles << face->cycles_[i];
        }

        // Set these new cycles to be the face boundary
        face->setCycles(newCycles);
        face->addMeToSpatialStarOf_(edge);
    }

    return true;
}

KeyVertex * VAC::cutFaceAtVertex_(KeyFace * face, double x, double y)
{
    KeyVertex * res = newKeyVertex(face->time());
    res->setPos(Eigen::Vector2d(x,y));
    Cycle newCycle(res);
    face->addCycle(newCycle);
    return res;
}

KeyVertex * VAC::cutEdgeAtVertex_(KeyEdge * edge, double s)
{
    double l = edge->geometry()->length();
    double eps = 1e-2;
    if(edge->isClosed())
    {
        std::vector<double> splitValues;
        splitValues.push_back(s);
        splitValues.push_back(s+l);
        SplitInfo info = cutEdgeAtVertices_(edge, splitValues);
        return info.newVertices[0];
    }
    else
    {
        if(eps<s && s<l-eps)
        {
            std::vector<double> splitValues;
            splitValues.push_back(0);
            splitValues.push_back(s);
            splitValues.push_back(l);
            SplitInfo info = cutEdgeAtVertices_(edge, splitValues);
            return info.newVertices[0];
        }
        else
        {
            return 0;
        }
    }
}

VAC::SplitInfo VAC::cutEdgeAtVertices_(KeyEdge * edgeToSplit, const std::vector<double> & splitValues)
{
    Time time = edgeToSplit->time();

    typedef SculptCurve::Curve<EdgeSample> SketchedEdge;

    // The return value
    SplitInfo res;
    res.oldEdge = edgeToSplit;

    // Split the curve
    std::vector<SketchedEdge,Eigen::aligned_allocator<SketchedEdge> > split = static_cast<LinearSpline *>(edgeToSplit->geometry())->curve().split(splitValues);

    // Get start node or create it in case of loop
    KeyVertex * startVertex;
    if(!edgeToSplit->isClosed())
    {
        startVertex = edgeToSplit->startVertex();
    }
    else
    {
        // Create new node
        EdgeSample v = split[0].start();
        startVertex = newKeyVertex(time);
        startVertex->setPos(Eigen::Vector2d(v.x(), v.y()));
        res.newVertices << startVertex;
    }

    // Keep this very first vertex for later
    KeyVertex * firstVertex = startVertex;

    // Create new nodes and edges
    QColor color = edgeToSplit->color();
    for(unsigned int j=0; j<split.size()-1; j++)
    {
        // Create new node
        EdgeSample v = split[j].end();
        KeyVertex * newVertex = newKeyVertex(time);
        newVertex->setPos(Eigen::Vector2d(v.x(), v.y()));
        res.newVertices << newVertex;

        // Create geometry out of it
        EdgeGeometry * geometry = new LinearSpline(split[j]);
        KeyEdge * iedge = newKeyEdge(time, startVertex, newVertex, geometry);
        iedge->setColor(color);
        res.newEdges << iedge;

        // Recurse
        startVertex = newVertex;
    }

    // Create geometry of last out of it
    EdgeGeometry * geometry = new LinearSpline(split.back());
    KeyVertex * endVertex;
    if(!edgeToSplit->isClosed())
        endVertex = edgeToSplit->endVertex();
    else
        endVertex = firstVertex;
    KeyEdge * iedge = newKeyEdge(time, startVertex, endVertex, geometry);
    iedge->setColor(color);
    res.newEdges << iedge;

    // Update star
    CellSet star = edgeToSplit->star();
    KeyFaceSet keyFaces =  star;
    InbetweenEdgeSet inbetweenEdges =  star;
    InbetweenFaceSet inbetweenFaces =  star;
    foreach(KeyFace * c, keyFaces)
    {
        c->updateBoundary(res.oldEdge, res.newEdges);
        c->geometryChanged_();
    }
    foreach(InbetweenEdge * c, inbetweenEdges)
    {
        c->updateBoundary(res.oldEdge, res.newEdges);
    }
    foreach(InbetweenFace * c, inbetweenFaces)
    {
        c->updateBoundary(res.oldEdge, res.newEdges);
    }

    // Now that all calls of c->boundary() are trustable, update cached star
    foreach(Cell * c, star)
    {
        c->removeMeFromStarOf_(res.oldEdge);
        c->addMeToStarOfBoundary_();
    }
    // Delete old edge
    edgeToSplit->destroy();

    // Return
    return res;
}


///////////////////////////////////////////////////////////////////////////
////////////////////               GLUING               ///////////////////

// convenient casting
/*
    KeyVertex * castedKeyVertex_;
    KeyEdge * castedKeyEdge_;
    KeyFace * castedKeyFace_;
    InbetweenVertex * castedInbetweenVertex_;
    InbetweenEdge * castedInbetweenEdge_;
    InbetweenFace * castedInbetweenFace_;
    void multicast_(Cell * c)
    {
        castedKeyVertex_ = c->toKeyVertex();
        castedKeyEdge_ = c->toKeyEdge();
        castedKeyFace_ = c->toKeyFace();
        castedInbetweenVertex_ = c->toInbetweenVertex();
        castedInbetweenEdge_ = c->toInbetweenEdge();
        castedInbetweenFace_ = c->toInbetweenFace();
    }


    void updateCellBoundary_ReplaceKeyVertexByAnother_(Cell * cell, KeyVertex * oldVertex, KeyVertex * newVertex)
    {
        multicast_
        K
        if(cell)
        if()
    }

    {

    }


    }
    */

void VAC::glue_(KeyVertex * v1, KeyVertex * v2)
{
    // make sure they have same time
    if(v1->time() != v2->time())
    {
        QMessageBox::information(0, QObject::tr("operation aborted"),
                                 QObject::tr("you can't glue two vertices not sharing the same time."));
        return;
    }

    // create new vertex
    KeyVertex * v3 = newKeyVertex(v1->time());
    v3->setPos(0.5*(v1->pos() + v2->pos()));

    // Update star
    CellSet star = v1->star();
    star.unite(v2->star());
    KeyEdgeSet keyEdges =  star;
    KeyFaceSet keyFaces =  star;
    InbetweenVertexSet inbetweenVertices =  star;
    InbetweenEdgeSet inbetweenEdges =  star;
    InbetweenFaceSet inbetweenFaces =  star;
    foreach(KeyEdge * c, keyEdges)
    {
        c->updateBoundary(v1,v3);
        c->updateBoundary(v2,v3);
        c->correctGeometry();
    }
    foreach(InbetweenVertex * c, inbetweenVertices)
    {
        c->updateBoundary(v1,v3);
        c->updateBoundary(v2,v3);
    }
    foreach(KeyFace * c, keyFaces)
    {
        c->updateBoundary(v1,v3);
        c->updateBoundary(v2,v3);
        c->geometryChanged_();
    }
    foreach(InbetweenEdge * c, inbetweenEdges)
    {
        c->updateBoundary(v1,v3);
        c->updateBoundary(v2,v3);
    }
    foreach(InbetweenFace * c, inbetweenFaces)
    {
        c->updateBoundary(v1,v3);
        c->updateBoundary(v2,v3);
    }

    // Now that all calls of c->boundary() are trustable, update cached star
    foreach(Cell * c, star)
    {
        c->removeMeFromStarOf_(v1);
        c->removeMeFromStarOf_(v2);
        c->addMeToStarOfBoundary_();
    }

    // delete glued vertices
    deleteCell(v1);
    deleteCell(v2);
}

namespace
{
bool haveSameOrientation(KeyEdge * e1, KeyEdge * e2)
{
    double l1 = e1->geometry()->length();
    Eigen::Vector2d u1 = e1->geometry()->der(0.5*l1);
    double l2 = e2->geometry()->length();
    Eigen::Vector2d u2 = e2->geometry()->der(0.5*l2);
    double dot = u1.dot(u2);
    if(dot>0)
        return !DevSettings::getBool("inverse direction"); // true by default
    else
        return DevSettings::getBool("inverse direction"); // false by default
}
}

void VAC::glue_(KeyEdge * e1, KeyEdge * e2)
{
    // make sure they have same time
    if(e1->time() != e2->time())
    {
        QMessageBox::information(0, QObject::tr("operation aborted"),
                                 QObject::tr("you can't glue two edges not sharing the same time."));
        return;
    }

    // make sure they have same topology
    if(e1->isClosed() != e2->isClosed())
    {
        QMessageBox::information(0, QObject::tr("operation aborted"),
                                 QObject::tr("you can't glue a closed edge with an open edge."));
        return;
    }

    // decide in what orientation to glue with simple heuristics
    glue_( KeyHalfedge(e1,true), KeyHalfedge(e2,haveSameOrientation(e1,e2)));
}

// assume h1 and h2 have same topology
void VAC::glue_(const KeyHalfedge &h1, const KeyHalfedge &h2)
{
    // glue end vertices
    if(!h1.isClosed())
    {
        if(h1.startVertex() != h2.startVertex())
            glue_(h1.startVertex(), h2.startVertex());
        if(h1.endVertex() != h2.endVertex())
            glue_(h1.endVertex(), h2.endVertex());
    }

    // Convenient data
    KeyEdge * e1 = h1.edge;
    KeyEdge * e2 = h2.edge;

    // compute new geometry
    SculptCurve::Curve<EdgeSample> & g1 = static_cast<LinearSpline*>(h1.edge->geometry())->curve();
    SculptCurve::Curve<EdgeSample> & g2 = static_cast<LinearSpline*>(h2.edge->geometry())->curve();
    double l1 = h1.edge->geometry()->length();
    double l2 = h2.edge->geometry()->length();
    std::vector<EdgeSample,Eigen::aligned_allocator<EdgeSample> > g3Vertices;
    int n1 = g1.size();
    int n2 = g2.size();
    int n = (n1+n2)/2 + 1;
    for(int i=0; i<n+1; i++)
    {
        double s1 = (double) i / (double) n * l1;
        if(!h1.side)
            s1 = l1 - s1;
        double s2 = (double) i / (double) n * l2;
        if(!h2.side)
            s2 = l2 - s2;

        EdgeSample es1 =  g1(s1);
        EdgeSample es2 =  g2(s2);
        g3Vertices << es1.lerp(0.5, es2);
    }
    SculptCurve::Curve<EdgeSample> g3;
    g3.setVertices(g3Vertices);
    if(h1.isClosed())
        g3.makeLoop();
    LinearSpline * ls3;
    ls3 = new LinearSpline(g3, h1.isClosed());

    // create new edge
    KeyEdge * e3;
    if(h1.isClosed())
        e3 = newKeyEdge(h1.time(), ls3);
    else
        e3 = newKeyEdge(h1.time(), h1.startVertex(), h1.endVertex(), ls3);
    KeyHalfedge h3(e3,true);


    // Update star
    CellSet star = e1->star();
    star.unite(e2->star());
    KeyFaceSet keyFaces =  star;
    InbetweenEdgeSet inbetweenEdges =  star;
    InbetweenFaceSet inbetweenFaces =  star;
    foreach(KeyFace * c, keyFaces)
    {
        c->updateBoundary(h1,h3);
        c->updateBoundary(h2,h3);
        c->geometryChanged_();
    }
    foreach(InbetweenEdge * c, inbetweenEdges)
    {
        c->updateBoundary(h1,h3);
        c->updateBoundary(h2,h3);
    }
    foreach(InbetweenFace * c, inbetweenFaces)
    {
        c->updateBoundary(h1,h3);
        c->updateBoundary(h2,h3);
    }

    // Now that all calls of c->boundary() are trustable, update cached star
    foreach(Cell * c, star)
    {
        c->removeMeFromStarOf_(e1);
        c->removeMeFromStarOf_(e2);
        c->addMeToStarOfBoundary_();
    }

    // set color
    QColor color1 = e1->color();
    QColor color2 = e2->color();
    e3->setColor(lerp(color1,color2,0.5));

    // delete glued edges
    deleteCell(e1);
    deleteCell(e2);
}

int VAC::nUses_(KeyVertex * v)
{
    int res = 0;

    KeyFaceSet incidentFaces = v->spatialStar();
    KeyEdgeSet incidentEdges = v->spatialStar();

    foreach(KeyFace * f, incidentFaces)
    {
        // count how many times f uses v
        for(int i=0; i<f->cycles_.size(); ++i)
        {
            if(f->cycles_[i].vertex_ == v) // Steiner vertex
                res++;

            for(int j=0; j<f->cycles_[i].size(); ++j)
            {
                if(f->cycles_[i][j].startVertex() == v)
                    res++;
            }
        }
    }

    foreach(KeyEdge * e, incidentEdges)
    {
        KeyFaceSet fs = e->spatialStar();
        if(fs.size() == 0) // otherwise, will be counted as a use by the face
        {
            if(e->startVertex() == v)
                res++;
            if(e->endVertex() == v)
                res++;
        }
    }

    return res;
}

int VAC::nUses_(KeyEdge * e)
{
    int res = 0;

    KeyFaceSet incidentFaces = e->spatialStar();
    foreach(KeyFace * f, incidentFaces)
    {
        // count how many times f uses e
        for(int i=0; i<f->cycles_.size(); ++i)
        {
            for(int j=0; j<f->cycles_[i].size(); ++j)
            {
                if(f->cycles_[i][j].edge == e)
                    res++;
            }
        }
    }

    return res;
}

void VAC::unglue_(KeyVertex * v)
{
    // compute uses
    int nUses = nUses_(v);
    //qDebug() << "n uses:" << nUses;

    if(nUses>1)
    {
        // Note: Unglue does not yet support incident inbetween cells
        //       As a workaround, we just delete all incident inbetween cells
        //       Later, we'll do something smarter
        CellSet inbetweenCells = v->temporalStar();
        deleteCells(inbetweenCells);

        // Unglue all incident edges
        KeyEdgeSet iEdges = v->spatialStar();
        foreach(KeyEdge * edge, iEdges)
            unglue_(edge);

        // Creates one duplicate vertex for each use
        KeyFaceSet incidentFaces = v->spatialStar();
        KeyEdgeSet incidentEdges = v->spatialStar();

        foreach(KeyFace * f, incidentFaces)
        {
            for(int i=0; i<f->cycles_.size(); ++i)
            {
                if(f->cycles_[i].vertex_ == v) // Steiner vertex
                {
                    // Create new vertex
                    KeyVertex * vNew = newKeyVertex(v->time());
                    vNew->pos_ = v->pos();

                    // Use it instead of original one
                    f->cycles_[i].vertex_ = vNew;
                    f->addMeToSpatialStarOf_(vNew);
                    f->removeMeFromSpatialStarOf_(v);
                }

                for(int j=0; j<f->cycles_[i].size(); ++j)
                {
                    if(f->cycles_[i][j].startVertex() == v)
                    {
                        // Create new vertex
                        KeyVertex * vNew = newKeyVertex(v->time());
                        vNew->pos_ = v->pos();

                        // Use it instead of original one
                        // The bracket here do "f->something_ = vNew" (done indirectly via the halfedges)
                        {
                            // for the two involved edges, use new vertex instead of the original one
                            // note: if v is used twice by e, you really want to change only the
                            //       appropriate one. The other will be created after

                            // Replace in f->cycles_[i][j]
                            KeyHalfedge & hAfter = f->cycles_[i].halfedges_[j];
                            if(hAfter.side)
                            {
                                hAfter.edge->startVertex_ = vNew;
                                hAfter.edge->addMeToSpatialStarOf_(vNew);
                                hAfter.edge->removeMeFromSpatialStarOf_(v);
                            }
                            else
                            {
                                hAfter.edge->endVertex_ = vNew;
                                hAfter.edge->addMeToSpatialStarOf_(vNew);
                                hAfter.edge->removeMeFromSpatialStarOf_(v);
                            }

                            // Replace in f->cycles_[i]["j-1"]
                            int jMinus1 = ( j==0 ? f->cycles_[i].size() - 1 : j-1 );
                            KeyHalfedge & hBefore = f->cycles_[i].halfedges_[jMinus1];
                            if(hBefore.side)
                            {
                                hBefore.edge->endVertex_ = vNew;
                                hBefore.edge->addMeToSpatialStarOf_(vNew);
                                hBefore.edge->removeMeFromSpatialStarOf_(v);
                            }
                            else
                            {
                                hBefore.edge->startVertex_ = vNew;
                                hBefore.edge->addMeToSpatialStarOf_(vNew);
                                hBefore.edge->removeMeFromSpatialStarOf_(v);
                            }
                        }
                        f->addMeToSpatialStarOf_(vNew);
                        f->removeMeFromSpatialStarOf_(v);
                    }
                }
            }

            // Recompute geometry
            f->geometryChanged_();
        }

        foreach(KeyEdge * e, incidentEdges)
        {
            KeyFaceSet fs = e->spatialStar();
            if(fs.size() == 0) // otherwise, will be counted as a use by the face
            {
                if(e->startVertex() == v)
                {
                    // Create new vertex
                    KeyVertex * vNew = newKeyVertex(v->time());
                    vNew->pos_ = v->pos();

                    // Use it instead of original one
                    e->startVertex_ = vNew;
                    e->addMeToSpatialStarOf_(vNew);
                    e->removeMeFromSpatialStarOf_(v);
                }
                if(e->endVertex() == v)
                {
                    // Create new vertex
                    KeyVertex * vNew = newKeyVertex(v->time());
                    vNew->pos_ = v->pos();

                    // Use it instead of original one
                    e->endVertex_ = vNew;
                    e->addMeToSpatialStarOf_(vNew);
                    e->removeMeFromSpatialStarOf_(v);
                }
            }
        }

        // Delete original vertex
        deleteCell(v);
    }
}


void VAC::unglue_(KeyEdge * e)
{
    // compute uses
    int nUses = nUses_(e);
    //qDebug() << "n uses:" << nUses;

    if(nUses>1)
    {
        // Note: Unglue does not yet support incident inbetween cells
        //       As a workaround, we just delete all incident inbetween cells
        //       Later, we'll do something smarter
        CellSet inbetweenCells = e->temporalStar();
        deleteCells(inbetweenCells);

        // Create one duplicate edge for each use
        KeyFaceSet incidentFaces = e->spatialStar();
        foreach(KeyFace * f, incidentFaces)
        {
            for(int i=0; i<f->cycles_.size(); ++i)
            {
                for(int j=0; j<f->cycles_[i].size(); ++j)
                {
                    if(f->cycles_[i][j].edge == e)
                    {
                        // duplicate edge
                        EdgeGeometry * geometryNew = e->geometry()->clone();
                        KeyEdge * eNew;
                        if(e->isClosed())
                            eNew = newKeyEdge(e->time(), geometryNew);
                        else
                            eNew = newKeyEdge(e->time(), e->startVertex(), e->endVertex(), geometryNew);

                        // set color
                        eNew->setColor(e->color());

                        // set duplicated edge as new boundary edge
                        f->cycles_[i].halfedges_[j].edge = eNew;
                        f->addMeToSpatialStarOf_(eNew);
                        f->removeMeFromSpatialStarOf_(e);
                    }
                }
            }

            // Recompute geometry
            f->geometryChanged_();
        }

        // Delete original edge
        deleteCell(e);
    }
}

bool VAC::uncut_(KeyVertex * v)
{
    // compute edge n usage, check it's not more than 2
    bool isSplittedLoop = false;
    KeyEdge * e1 = 0;
    KeyEdge * e2 = 0;
    KeyEdgeSet incidentEdges = v->spatialStar();
    if(incidentEdges.size() == 0)
    {
        // Then can be uncut if it is a steiner vertex of one face, and one face only
        KeyFaceSet incidentFaces = v->spatialStar();
        bool found = false;
        KeyFace * foundFace = 0;
        int foundI = -1;
        foreach(KeyFace * f, incidentFaces)
        {
            for(int i=0; i<f->cycles_.size(); ++i)
            {
                if(f->cycles_[i].vertex_ == v) // Steiner vertex
                {
                    if(found)
                    {
                        //qDebug() << "Abort: Steiner vertex more than once, can't uncut here";
                        return false;
                    }
                    else
                    {
                        found = true;
                        foundFace = f;
                        foundI = i;
                    }
                }
            }

            // Recompute geometry
            f->geometryChanged_();
        }

        if(found == true)
        {
            // remove steiner vertex from cycles
            QList<Cycle> newCycles;
            for(int i=0; i<foundFace->cycles_.size(); ++i)
                if(i != foundI)
                    newCycles << foundFace->cycles_[i];

            // update face
            foundFace->cycles_ = newCycles;
            foundFace->removeMeFromSpatialStarOf_(v);

            // delete vertex
            deleteCell(v);

            // return
            return true;
        }
        else
        {
            //qDebug() << "Abort: vertex has no star cells";
            return false;
        }
    }
    else if(incidentEdges.size() == 1)
    {
        e1 = *incidentEdges.begin();
        isSplittedLoop = true;
        if(e1->startVertex() != e1->endVertex())
        {
            //qDebug() << "Uncut abort: only one incident edge that do not loop";
            return false;
        }
    }
    else if(incidentEdges.size() == 2)
    {
        e1 = *incidentEdges.begin();
        e2 = *(++incidentEdges.begin());
        if( (e1->startVertex() == e1->endVertex()) || (e2->startVertex() == e2->endVertex()))
        {
            //qDebug() << "Uncut abort: more than two incident edges";
            return false;
        }
    }
    else
    {
        //qDebug() << "Uncut abort: more than two incident edges";
        return false;
    }

    // From here, we already left the method if incidentEdges.size() == 0.
    // hence, the following only concerns the case where the vertex has at least
    // one incident edge

    // check that removing this vertex is compatible with incident faces
    KeyFaceSet incidentFaces = v->spatialStar();
    foreach(KeyFace * f, incidentFaces)
    {
        // check that it can be removed
        for(int i=0; i<f->cycles_.size(); ++i)
        {
            if(f->cycles_[i].vertex_ == v) // Steiner vertex
            {
                //qDebug() << "Vertex used as Steiner vertex: can't uncut here";
                return false;
            }

            for(int j=0; j<f->cycles_[i].size(); ++j)
            {
                if(f->cycles_[i][j].startVertex() == v)
                {
                    // used here
                    if(isSplittedLoop)
                    {
                        // check that it is alone: in this case it will be replace
                        if(f->cycles_[i].size() != 1)
                        {
                            // TODO: actually, could cut if the other halfedges are also
                            // using the same edge (EDIT after EDIT: same direction (no switch backs))
                            // EDIT after TODO: actually... not. What would be done in this case?
                            // the edge is a splitted loop. Hence, the result of uncutting would give a loop.
                            // Either:
                            //   - we allow to repeat a pure loop inside a single cycle
                            //   - we can't uncut, since removing
                            // To keep rendering/winding rules unchanged
                            //qDebug() << "Split loop involved with other halfedges in cycle: can't uncut here";
                            return false;
                        }
                    }
                    else
                    {
                        // check that the same edge is not repeated
                        int jMinus1 = ( j==0 ? f->cycles_[i].size() - 1 : j-1 );
                        if(f->cycles_[i][jMinus1].edge == f->cycles_[i][j].edge)
                        {
                            //qDebug() << "Cycle switching back direction at vertex: can't uncut here";
                            return false;
                        }
                    }
                }
            }
        }

        // Recompute geometry
        f->geometryChanged_();
    }

    // We're OK now, just do it :-)

    if(isSplittedLoop)
    {
        // transform splitted loop into pure loop
        e1->startVertex_ = 0;
        e1->endVertex_ = 0;
        e1->removeMeFromSpatialStarOf_(v);
        e1->geometry()->makeLoop();

        // update incident faces
        foreach(KeyFace * f, incidentFaces)
        {
            for(int i=0; i<f->cycles_.size(); ++i)
            {
                for(int j=0; j<f->cycles_[i].size(); ++j)
                {
                    if(f->cycles_[i][j].edge == e1)
                    {
                        f->removeMeFromSpatialStarOf_(v);
                    }
                }
            }

            // Recompute geometry
            f->geometryChanged_();
        }

        // delete vertex
        deleteCell(v);
    }
    else
    {
        // get orientation: h1 -> v -> h2
        KeyHalfedge h1(e1, (e1->endVertex() == v ? true : false) );
        KeyHalfedge h2(e2, (e2->startVertex() == v ? true : false) );

        // create equivalent edge/halfedge
        // [... ; h = (e,true) ; ...]  <=>  [...;h1;h2;...]

        // compute new geometry
        SculptCurve::Curve<EdgeSample> & g1 = static_cast<LinearSpline*>(e1->geometry())->curve();
        SculptCurve::Curve<EdgeSample> & g2 = static_cast<LinearSpline*>(e2->geometry())->curve();
        std::vector<EdgeSample,Eigen::aligned_allocator<EdgeSample> > g3Vertices;
        int n1 = g1.size();
        int n2 = g2.size();
        if(h1.side)
        {
            for(int i=0; i<n1; ++i)
                g3Vertices << g1[i];
        }
        else
        {
            for(int i=n1-1; i>=0; --i)
                g3Vertices << g1[i];
        }
        if(h2.side)
        {
            for(int i=1; i<n2; ++i)
                g3Vertices << g2[i];
        }
        else
        {
            for(int i=n2-2; i>=0; --i)
                g3Vertices << g2[i];
        }
        SculptCurve::Curve<EdgeSample> g3;
        g3.setVertices(g3Vertices);
        LinearSpline * ls3 = new LinearSpline(g3, false);

        // create new edge
        KeyEdge * e = newKeyEdge(v->time(), h1.startVertex(), h2.endVertex(), ls3);
        QColor color1 = e1->color();
        QColor color2 = e2->color();
        e->setColor(lerp(color1,color2,0.5));

        // update incident faces
        foreach(KeyFace * f, incidentFaces)
        {
            for(int i=0; i<f->cycles_.size(); ++i)
            {
                Cycle newCycle;
                bool cycleHasChanged = false;

                for(int j=0; j<f->cycles_[i].size(); ++j)
                {
                    if(f->cycles_[i][j].edge == e1)
                    {
                        // do nothing
                    }
                    else if (f->cycles_[i][j].edge == e2)
                    {
                        if(f->cycles_[i][j].side == h2.side)
                        {
                            newCycle.halfedges_ << KeyHalfedge(e,true);
                        }
                        else
                        {
                            newCycle.halfedges_ << KeyHalfedge(e,false);
                        }
                        cycleHasChanged = true;
                    }
                    else
                    {
                        newCycle.halfedges_ << f->cycles_[i][j];
                    }
                }

                if(cycleHasChanged)
                {
                    f->cycles_[i] = newCycle;
                    f->addMeToSpatialStarOf_(e);
                    f->removeMeFromSpatialStarOf_(e1);
                    f->removeMeFromSpatialStarOf_(e2);
                    f->removeMeFromSpatialStarOf_(v);
                }
            }

            // Recompute geometry
            f->geometryChanged_();
        }

        // delete vertex
        deleteCell(v);
    }

    return true;
}

bool VAC::uncut_(KeyEdge * e)
{
    // Compute number of uses
    int nUses = nUses_(e);
    if(nUses < 2)
    {
        //qDebug() << "less than 2 faces are incident to this edge: nothing to uncut";
        return false;
    }
    else if (nUses > 2)
    {
        //qDebug() << "more than 2 faces are incident to this edge: cannot uncut";
        return false;
    }

    // in case the edge is a loop
    if(e->isClosed())
    {
        // get incident faces
        KeyFaceSet incidentFaces = e->spatialStar();

        // two cases: either the two usages are from the same face, or from two different faces
        if(incidentFaces.size() == 1)
        {
            // in case they are from the same face, just remove the two cycles
            KeyFace * f = *incidentFaces.begin();
            QList<Cycle> newCycles;
            for(int i=0; i<f->cycles_.size(); ++i)
            {
                if( (f->cycles_[i].type() == Cycle::SingleVertex) ||
                        (f->cycles_[i][0].edge != e) )
                    newCycles << f->cycles_[i];
            }
            f->cycles_ = newCycles;
            f->removeMeFromSpatialStarOf_(e);

            // Recompute geometry
            f->geometryChanged_();

            // and delete the edge
            deleteCell(e);
        }
        else if (incidentFaces.size() == 2)
        {
            // in case they are from two different faces, remove the cycle
            // from each, and combine topology in a single face

            // remove cycles
            KeyFace * f1 = *incidentFaces.begin();
            KeyFace * f2 = *(++incidentFaces.begin());

            // get all cycles of f1 except e
            QList<Cycle> newCycles;
            for(int i=0; i<f1->cycles_.size(); ++i)
            {
                if( (f1->cycles_[i].type() == Cycle::SingleVertex) ||
                        (f1->cycles_[i][0].edge != e) )
                    newCycles << f1->cycles_[i];
            }

            // get all cycles of f2 except e
            for(int i=0; i<f2->cycles_.size(); ++i)
            {
                if( (f2->cycles_[i].type() == Cycle::SingleVertex) ||
                        (f2->cycles_[i][0].edge != e) )
                    newCycles << f2->cycles_[i];
            }

            // delete f2
            deleteCell(f2);

            // update f1
            f1->cycles_ = newCycles;
            f1->removeMeFromSpatialStarOf_(e);
            foreach(Cell * c, f1->spatialBoundary())
                f1->addMeToSpatialStarOf_(c);

            // Recompute geometry
            f1->geometryChanged_();

            // delete e
            deleteCell(e);
        }
        else
        {
            // can't happen, we know nUses == 2, hence incidentFaces.size == 1 or 2
        }
    }
    // In case the edge is an open edge
    else
    {

        //////////////////////////////////////////////////////////////////////////////////
        //          Compute newCycles, cycle1 and cycle2, delete f2 if any              //
        //////////////////////////////////////////////////////////////////////////////////

        // The new cycles
        QList<Cycle> newCycles;

        // The two cycles to merge
        Cycle cycle1;
        Cycle cycle2;

        // Get incident faces
        KeyFaceSet incidentFaces = e->spatialStar();
        if(incidentFaces.size() == 1)
        {
            // Either One face One cycle
            // Or     One face Two cycles
            KeyFace * f = *incidentFaces.begin();
            for(int i=0; i<f->cycles_.size(); ++i)
            {
                // check if e belongs to this cycle
                int eBelongsToCycle = 0;
                int j1 = -1; // if it does, here are the indices
                int j2 = -1;
                int n = f->cycles_[i].size();
                for(int j=0; j<n; ++j)
                    if(f->cycles_[i][j].edge == e)
                    {
                        eBelongsToCycle++;
                        if(j1 == -1)
                            j1 = j;
                        else
                            j2 = j;
                    }

                // do something accordingly
                if(eBelongsToCycle == 0)
                {
                    // if e doesn't belong to cycle, just add cycle to newCycles
                    newCycles << f->cycles_[i];
                }
                else if(eBelongsToCycle == 1)
                {
                    // If e belongs once to cycle
                    // ==> One face Two cycles

                    // Get the cycle to which we copy halfedges
                    Cycle & cycle = ( cycle1.size() ? cycle2 : cycle1 );

                    // Copy all halfedges of f->cycles_[i], except e, into cycle1 or cycle2.
                    int j1Plus1 = ( j1==n-1 ? 0 : j1+1);
                    for(int j=j1Plus1; j != j1; j = (j==n-1 ? 0 : j+1) )
                        cycle.halfedges_ << f->cycles_[i][j];

                    // Handle Steiner vertex case
                    if(cycle.size() == 0)
                    {
                        // This means that f->cycles_[i].size() = 1
                        // But we know e belong to the cycle and is open
                        // This means e is a splitted loop
                        // This means e->left() == e->right().
                        //
                        // Cycle must be of type Single Vertex

                        cycle.vertex_ = e->startVertex();
                    }
                }
                else if(eBelongsToCycle == 2)
                {
                    // if e belongs twice to the same cycle
                    // ==> One face One cycle

                    // Copy all halfedges of f->cycles_[i], except e, into cycle1 and cycle2.

                    // Get indices where the cycles start
                    int j1Plus1 = ( j1==n-1 ? 0 : j1+1);
                    int j2Plus1 = ( j2==n-1 ? 0 : j2+1);

                    // Cycle 1
                    for(int j=j1Plus1; j != j2; j = (j==n-1 ? 0 : j+1) )
                        cycle1.halfedges_ << f->cycles_[i][j];
                    // Handle Steiner vertex case
                    if(cycle1.size() == 0)
                    {
                        // means j1+1 = j2
                        // must add vertex between j1 and j2
                        cycle1.vertex_ = f->cycles_[i][j1].endVertex();
                    }

                    // Cycle 2
                    for(int j=j2Plus1; j != j1; j = (j==n-1 ? 0 : j+1) )
                        cycle2.halfedges_ << f->cycles_[i][j];
                    // Handle Steiner vertex case
                    if(cycle2.size() == 0)
                    {
                        // means j2+1 = j1
                        // must add vertex between j2 and j1
                        cycle2.vertex_ = f->cycles_[i][j2].endVertex();
                    }
                }
                else
                {
                    // can't happen
                }
            }

            // Recompute geometry
            f->geometryChanged_();
        }
        else if (incidentFaces.size() == 2)
        {
            // ==> Two faces Two cycles

            // Face 1: append cycles to newCycles or Cycle 1
            KeyFace * f1 = *incidentFaces.begin();
            for(int i=0; i<f1->cycles_.size(); ++i)
            {
                // check if (and where) e belongs to this cycle
                int j1 = -1;
                int n1 = f1->cycles_[i].size();
                for(int j=0; j<n1; ++j)
                    if(f1->cycles_[i][j].edge == e)
                        j1 = j;

                // do something accordingly
                if(j1 == -1) // if doesn't belong to cycle
                {
                    // add cycle to newCycles
                    newCycles << f1->cycles_[i];
                }
                else // if belongs to cycle
                {
                    // Copy all halfedges of f1->cycles_[i], except e, into cycle1
                    int j1Plus1 = ( j1==n1-1 ? 0 : j1+1);
                    for(int j=j1Plus1; j != j1; j = (j==n1-1 ? 0 : j+1) )
                        cycle1.halfedges_ << f1->cycles_[i][j];

                    // Handle Steiner vertex case
                    if(cycle1.size() == 0)
                        cycle1.vertex_ = e->startVertex();
                }
            }

            // Face 2: append cycles to newCycles or Cycle 2
            KeyFace * f2 = *(++incidentFaces.begin());
            for(int i=0; i<f2->cycles_.size(); ++i)
            {
                // check if (and where) e belongs to this cycle
                int j2 = -1;
                int n2 = f2->cycles_[i].size();
                for(int j=0; j<n2; ++j)
                    if(f2->cycles_[i][j].edge == e)
                        j2 = j;

                // do something accordingly
                if(j2 == -1) // if doesn't belong to cycle
                {
                    // add cycle to newCycles
                    newCycles << f2->cycles_[i];
                }
                else // if belongs to cycle
                {
                    // Copy all halfedges of f1->cycles_[i], except e, into cycle1
                    int j2Plus1 = ( j2==n2-1 ? 0 : j2+1);
                    for(int j=j2Plus1; j != j2; j = (j==n2-1 ? 0 : j+1) )
                        cycle2.halfedges_ << f2->cycles_[i][j];

                    // Handle Steiner vertex case
                    if(cycle2.size() == 0)
                        cycle2.vertex_ = e->startVertex();
                }
            }

            // Set color
            QColor color1 = f1->color();
            QColor color2 = f2->color();
            f1->setColor(lerp(color1,color2,0.5));

            // Delete Face 2
            deleteCell(f2);

            // Recompute geometry
            f1->geometryChanged_();
        }


        //////////////////////////////////////////////////////////////////////////////////
        //    Decide on what to do with cycle1 and cycle2, add result to newCycles      //
        //////////////////////////////////////////////////////////////////////////////////

        // Ensure that the cycles are valid
        if(!cycle1.isValid() || !cycle2.isValid())
        {
            //qDebug() << "Uncut abort: cycle 1 or cycle 2 is invalid for an unknown reason";
            return false;
        }

        // Handle Steiner vertices
        if(cycle1.type() == Cycle::SingleVertex)
        {
            // If Cycle 1 is a Steiner vertex, then add Cycle 2 in any cases,
            // and add Cycle 1 only if not contained in Cycle 2
            if(cycle2.cells().contains(cycle1.singleVertex()))
                newCycles << cycle2;
            else
                newCycles << cycle1 << cycle2;
        }
        else if(cycle2.type() == Cycle::SingleVertex)
        {
            // If Cycle 1 is a Steiner vertex, then add Cycle 2 in any cases,
            // and add Cycle 1 only if not contained in Cycle 2
            if(cycle1.cells().contains(cycle2.singleVertex()))
                newCycles << cycle1;
            else
                newCycles << cycle2 << cycle1;
        }

        // Handle cases where none of cycle 1 or cycle 2 are Steiner vertices
        else
        {
            // Check if can or must be merge
            KeyVertex * startCycle1 = cycle1.halfedges_.front().startVertex();
            KeyVertex * endCycle1 = cycle1.halfedges_.back().endVertex();
            KeyVertex * startCycle2 = cycle2.halfedges_.front().startVertex();
            KeyVertex * endCycle2 = cycle2.halfedges_.back().endVertex();

            // if the two cycles are already valid, great! Don't touch them
            if( (endCycle1 == startCycle1) && (endCycle2 == startCycle2) )
            {
                newCycles << cycle1 << cycle2;
            }
            // Otherwise need to combine them
            else if( (endCycle1 == startCycle2) && (endCycle2 == startCycle1) )
            {
                // if already compatible, great! just append them
                for(int j=0; j<cycle2.size(); ++j)
                    cycle1.halfedges_ << cycle2[j];

                newCycles << cycle1;
            }
            else if( (endCycle1 == endCycle2) && (startCycle2 == startCycle1) )
            {
                // in this case, we just have to reverse one of them, no big deal.
                for(int j=cycle2.size()-1; j>=0; --j)
                    cycle1.halfedges_ << cycle2[j].opposite();

                newCycles << cycle1;
            }
            else
            {
                // woops, this shouldn't happen
                //qDebug() << "Uncut abort: can't combine cycle 1 and 2 for some unknown reason";
                return false;
            }
        }


        //////////////////////////////////////////////////////////////////////////////////
        //    Update merged face                                                    //
        //////////////////////////////////////////////////////////////////////////////////

        // update topology
        KeyFace * f = *incidentFaces.begin();
        f->cycles_ = newCycles;
        f->removeMeFromSpatialStarOf_(e);
        foreach(Cell * c, f->spatialBoundary())
            f->addMeToSpatialStarOf_(c);

        // update z-ordering
        zOrdering_.removeCell(f);
        zOrdering_.insertCell(f);

        // Recompute geometry
        f->geometryChanged_();

        // delete e
        deleteCell(e);
    }

    return true;
}

void VAC::insertSketchedEdgeInVAC()
{
    double tolerance = global()->snapThreshold();
    double toleranceEpsilon = 1e-2;
    if( (tolerance < toleranceEpsilon) || !(global()->snapMode()) )
        tolerance = 1e-2;
    insertSketchedEdgeInVAC(tolerance);
}

void VAC::insertSketchedEdgeInVAC(double tolerance, bool useFaceToConsiderForCutting)
{
    // --------------------------------------------------------------------
    // ---------------------- Input Variables -----------------------------
    // --------------------------------------------------------------------

    bool intersectWithSelf = global()->planarMapMode();
    bool intersectWithOthers = global()->planarMapMode();

    // --------------------------------------------------------------------
    // ----------------- Compute dirty intersections ----------------------
    // --------------------------------------------------------------------

    typedef SculptCurve::Curve<EdgeSample> SketchedEdge;
    std::vector<             SculptCurve::Intersection  > selfIntersections;
    std::vector< std::vector<SculptCurve::Intersection> > othersIntersections;

    // Lengths of the sketched edge and existing ("others") edges
    double lSelf = sketchedEdge_->length(); // compute it now
    std::vector<double> lOthers;            // will be computed inside the loop

    // Store geometry of existing edges as a "SketchedEdge"
    std::vector< SculptCurve::Curve<EdgeSample>,Eigen::aligned_allocator<SculptCurve::Curve<EdgeSample> >  > sketchedEdges; // sketchedEdges.size() == nEdges.

    // Compute intersections with self
    if(intersectWithSelf)
        selfIntersections = sketchedEdge_->curve().selfIntersections(tolerance);

    // Keyframe existing inbetween edge that intersect with sketched edge
    if(intersectWithOthers)
    {
        InbetweenEdgeSet inbetweenEdges;
        foreach(Cell * cell, cells())
        {
            InbetweenEdge * sedge = cell->toInbetweenEdge();
            if(sedge && sedge->exists(timeInteractivity_))
                inbetweenEdges << sedge;
        }
        foreach(InbetweenEdge * sedge, inbetweenEdges)
        {
            // Get sampling as a QList of EdgeSamples
            QList<EdgeSample> sampling = sedge->getSampling(timeInteractivity_);

            // Convert sampling to a std::vector of EdgeSamples
            std::vector<EdgeSample,Eigen::aligned_allocator<EdgeSample> > stdSampling;
            for(int i=0; i<sampling.size(); ++i)
                stdSampling << sampling[i];

            // Convert sampling to a SculptCurve::Curve<EdgeSample>
            SculptCurve::Curve<EdgeSample> sketchedEdge;
            sketchedEdge.setVertices(stdSampling);

            // Compute intersections
            std::vector<SculptCurve::Intersection> intersections = sketchedEdge_->curve().intersections(sketchedEdge, tolerance);

            // Keyframe edge if there are some intersections
            if(intersections.size() > 0)
                keyframe_(sedge, timeInteractivity_);
        }
    }

    // Compute intersections with others
    KeyEdgeList iedgesBefore; // the list of edges before they are split by the newly sketched edge
    int nEdges = 0;               // the number of them
    if(intersectWithOthers)
    {
        // Get existing edges
        iedgesBefore = instantEdges(timeInteractivity_);
        nEdges = iedgesBefore.size();

        // For each of them, compute intersections with sketched edge
        foreach (KeyEdge * iedge, iedgesBefore)
        {
            // Convert geometry of instant edge to a SketchedEdge
            EdgeGeometry * geometry = iedge->geometry();
            LinearSpline * linearSpline = dynamic_cast<LinearSpline *>(geometry);
            if(linearSpline)
            {
                sketchedEdges << linearSpline->curve() ;
            }
            else
            {
                QList<Eigen::Vector2d> eigenSampling = geometry->sampling(ds_);
                std::vector<EdgeSample,Eigen::aligned_allocator<EdgeSample> > vertices;
                for(int i=0; i<eigenSampling.size(); ++i)
                    vertices << EdgeSample(eigenSampling[i][0], eigenSampling[i][1], 10); // todo: get actual width
                sketchedEdges << SculptCurve::Curve<EdgeSample>();
                sketchedEdges.back().setVertices(vertices);
            }

            // Compute intersections
            othersIntersections << sketchedEdge_->curve().intersections(sketchedEdges.back(), tolerance);

            // Store length
            lOthers << sketchedEdges.back().length();
        }
    }


    // --------------------------------------------------------------------
    // ----------------- Compute dirty split values -----------------------
    // --------------------------------------------------------------------

    // Convert dirty intersections to dirty (= with possible duplicates) split values
    std::vector<             double  > selfSplitValues_dirty;
    std::vector< std::vector<double> > othersSplitValues_dirty(nEdges, std::vector<double>() );

    // Self split values due to self-intersections + endpoints of sketched edge
    // Note: detecting if the sketched edge is a loop is done at a later stage
    for(auto intersection : selfIntersections)
        selfSplitValues_dirty << intersection.s << intersection.t;
    selfSplitValues_dirty << 0.0 << lSelf;

    // Split values (both self and others) due to intersections with existing edges
    for(int i=0; i<nEdges; ++i)
    {
        for(auto intersection : othersIntersections[i])
        {
            selfSplitValues_dirty << intersection.s;
            othersSplitValues_dirty[i] << intersection.t;
        }
    }

    // Sort dirty split values
    std::sort(selfSplitValues_dirty.begin(),
              selfSplitValues_dirty.end());

    for(auto & otherSplitValues : othersSplitValues_dirty)
    {
        std::sort(otherSplitValues.begin(),
                  otherSplitValues.end());
    }

#if MYDEBUG
    {
        /////////  Print split values so far  /////////
        std::cout << "Raw split values:" << std::endl;
        std::cout << "  Self split values = [ ";
        for(double s : selfSplitValues_dirty)
            std::cout << s << " ";
        std::cout << "]" << std::endl;
        std::cout << "  Others split values =" << std::endl;
        for(auto splitValues : othersSplitValues_dirty)
        {
            std::cout << "    [ ";
            for(double s : splitValues)
                std::cout << s << " ";
            std::cout << "]" << std::endl;
        }
        std::cout << std::endl;
    }
#endif


    // --------------------------------------------------------------------
    // ---------------- Remove duplicated split values --------------------
    // --------------------------------------------------------------------

    // Remove duplicates
    std::vector<             double  > selfSplitValues;
    std::vector< std::vector<double> > othersSplitValues(nEdges, std::vector<double>() );

    // Convenient struct to avoid code duplication (almost same code for self/others intersections)
    struct SplitValuesToClean
    {
        SplitValuesToClean(std::vector<double> & dirty, std::vector<double> & clean, double l, bool isSelf, bool isClosed) :
            dirty(dirty), clean(clean), l(l), isSelf(isSelf), isClosed(isClosed) {}

        std::vector<double> & dirty;
        std::vector<double> & clean;
        double l;
        bool isSelf; // a flag to handle specific code only applying to self intersections
        bool isClosed;
    };

    // Build vector of split values to clean
    std::vector<SplitValuesToClean> toClean;
    toClean.emplace_back(selfSplitValues_dirty, selfSplitValues, lSelf, true, false); // self split values

    // For each of them, compute intersections with sketched edge
    {
        int i = 0;
        foreach (KeyEdge * iedge, iedgesBefore)
        {
            // Avoid cleaning non-intersected existing edges
            if(othersSplitValues_dirty[i].size() > 0)
                toClean.emplace_back(othersSplitValues_dirty[i], othersSplitValues[i], lOthers[i], false, iedge->isClosed());
            ++i;
        }
    }

    // Clean all split values
    // This is done by "clustering" together nearby split values (within specified tolerance)
    // and replacing them by a single new split value, averaging the clustered old values
    for(auto splitValues : toClean)
    {
        // Safety check: nothing to clean if no split values
        if(splitValues.dirty.size() == 0)
            continue;

        // Variables to cluster split values together
        int nMean = 0;  // current number of split values in current cluster
        double sum = 0; // current sum    of split values in current cluster

        // Get and add first split value
        double firstSplitValue;
        if(splitValues.isClosed)
        {
            //qDebug() << "isClosed";
            firstSplitValue = splitValues.dirty.front();
        }
        else
            firstSplitValue = 0;
        splitValues.clean << firstSplitValue;

        // Get last split value
        double lastSplitValue;
        if(splitValues.isClosed)
            lastSplitValue = firstSplitValue + splitValues.l;
        else
            lastSplitValue = splitValues.l;

        // Main loop over all split values
        for(double s : splitValues.dirty)
        {
            // ignore all split values too close to start split value
            if(s < firstSplitValue + tolerance)
                continue;

            // ignore all split values too close to end split value
            if(s > lastSplitValue - tolerance)
                break;

            if(nMean == 0) // Add first split value to first cluster
            {
                nMean = 1;
                sum = s;
            }
            else           // Check if next split value must be appended to the current cluster
            {              // or inserted as the first split value of a new cluster
                // the mean value that would be added if we stop contributing to this cluster
                double mean = sum/nMean;

                // test if adding the next value would infer a distance greater than tol or not
                if(s > mean + tolerance)
                {
                    // If yes, then insert the mean
                    splitValues.clean << mean;

                    // And recurse: create a new cluster
                    nMean = 1;
                    sum = s;
                }
                else
                {
                    // Contribute to the mean
                    nMean++;
                    sum += s;
                }
            }
        }
        // No more split values to process, add the one from last cluster
        if(nMean > 0)
            splitValues.clean << sum/nMean;

        // Add last split value
        splitValues.clean << lastSplitValue;
    }

#if MYDEBUG
    /////////  Print cleaned split values  /////////
    std::cout << "Cleaning split values:" << std::endl;
    std::cout << "  Self split values = [ ";
    for(double s : selfSplitValues)
        std::cout << s << " ";
    std::cout << "] -- length = " << sketchedEdge_->length() << std::endl;
    std::cout << "  Others split values =" << std::endl;
    int i1 = 0;
    for(auto splitValues : othersSplitValues)
    {
        std::cout << "    [ ";
        for(double s : splitValues)
            std::cout << s << " ";
        std::cout << "] -- length = " << sketchedEdges[i1].length() << std::endl;
        i1++;
    }
    std::cout << std::endl;
#endif


    // --------------------------------------------------------------------
    // ---- Compute node positions corresponding to split values ----------
    // --------------------------------------------------------------------

    struct SplitNodes {

        int size() const { return nSelf + nExisting; }
        EdgeSample operator[] (int i)
        {
            if(i<nSelf)
                return self[i];
            else
            {
                i -= nSelf;
                return existing[i];
            }
        }

        int nSelf;
        std::vector<EdgeSample,Eigen::aligned_allocator<EdgeSample> > self;

        int nExisting;
        std::vector<EdgeSample,Eigen::aligned_allocator<EdgeSample> > existing;
        std::vector<KeyVertex *> existingNodes;

    } splitNodes;

    // Nodes created via selfIntersections
    splitNodes.nSelf = selfSplitValues.size();
    splitNodes.self.reserve(splitNodes.nSelf);
    for(int i=0; i<splitNodes.nSelf; ++i)
        splitNodes.self.push_back(sketchedEdge_->curve()(selfSplitValues[i]));

    // Existing nodes, at the end of intersected other curves
    { // create scope so that i stays local
        int i=0;
        foreach(KeyEdge * iedge, iedgesBefore) // TODO: generalize to Animated Nodes as well
        {
            if(othersSplitValues[i].size() > 0 && !iedge->isClosed())
            {
                // todo: be careful!! Potentially add several times the same node here!!!
                splitNodes.existing << sketchedEdges[i].start();
                splitNodes.existingNodes << iedge->startVertex();

                splitNodes.existing << sketchedEdges[i].end();
                splitNodes.existingNodes << iedge->endVertex();
            }
            i++;
        }
    }

    // Existing nodes, close to end nodes.
    {
        EdgeSample startVertex = sketchedEdge_->curve().start();
        EdgeSample endVertex = sketchedEdge_->curve().end();
        foreach(KeyVertex * v, instantVertices(timeInteractivity_))
        {
            // todo: be careful!! Potentially add several times the same node here!!!
            EdgeSample sv = startVertex;
            sv.setX(v->pos()[0]);
            sv.setY(v->pos()[1]);
            if(sv.distanceTo(startVertex) < tolerance)
            {
                splitNodes.existing << sv;
                splitNodes.existingNodes << v;
            }
            else
            {
                sv = endVertex;
                sv.setX(v->pos()[0]);
                sv.setY(v->pos()[1]);
                if(sv.distanceTo(endVertex) < tolerance)
                {
                    splitNodes.existing << sv;
                    splitNodes.existingNodes << v;
                }
            }
        }
    }

    // Vertices created via intersection with other curves
    // Warning: this step destroy some edges in iedgesBefore, but without removing
    // them form
    {
        int i = 0;
        foreach(KeyEdge * oldEdge, iedgesBefore)
        {
            if(othersSplitValues[i].size() > 2 || (oldEdge->isClosed() && othersSplitValues[i].size() > 1))
                // avoid splitting if the cleaned split values are [0,l]
                // unless it's a loop and split values can be [s,s+l]
            {
                // Split the edge
                SplitInfo info = cutEdgeAtVertices_(oldEdge, othersSplitValues[i]);
                foreach(KeyVertex * ivertex, info.newVertices)
                {
                    splitNodes.existing << EdgeSample(ivertex->pos()[0], ivertex->pos()[1]);
                    splitNodes.existingNodes << ivertex;
                }
            }
            i++;
        }

        // From this point, iedgesBefore must not be used, since some
        // of its edges are deleted
        iedgesBefore.clear();
    }
    splitNodes.nExisting = splitNodes.existing.size();


#if MYDEBUG
    /////////  Print position of BasivVertex  /////////
    std::cout << "  Positions = [ ";
    for(int i=0; i<splitNodes.size(); ++i)
        std::cout << "(" << splitNodes[i].x() << "," << splitNodes[i].y() << ") ";
    std::cout << "]" << std::endl;
    std::cout << std::endl;
#endif


    // --------------------------------------------------------------------
    // ------------------- Create 2D clustering graph ---------------------
    // --------------------------------------------------------------------

    // Create graph where nodes are linked by an edge iff their 2D euclidean distance is < tol
    // The graph is represented as an adjacency list
    // Initialize adj list
    int nSplit = splitNodes.size();
    std::vector< std::vector<int> > neighbours(nSplit, std::vector<int>());

    // feed adj list: here, room for optimization from n^2 to n log(n) by sorting and swipe line algos
    // in practice, n is rarely gonna exceeding 10, so... not a serious concern for now
    for(int i=0; i<nSplit; ++i)
        for(int j=i+1; j<nSplit; ++j)
        {
            double d = splitNodes[i].distanceTo(splitNodes[j]);
            if(d<tolerance)
            {
                neighbours[i].push_back(j);
                neighbours[j].push_back(i);
            }
        }

    // Aux structure associated with each cluster.
    // It creates a new Node() if no existing node exists in the cluster.
    // Then, for each i in the clusters, return the corresponding Node *.
    //
    // Note: the comment above is obsolete, now the cluster itself has no
    //       logic, just store the data, and what is written above is not
    //       done by the cluster but by the algorithm just after
    //
    struct Cluster
    {
        // members
        std::vector<int> indices;

        // member methods
        int size() const { return indices.size(); }
        Cluster & operator<<(int i) { indices << i; return *this; }
    };

    // Compute connected components: those are the clusters
    std::vector< Cluster > clusters;
    std::vector<bool> marked(nSplit, false);
    for(int startNode=0; startNode<nSplit; ++startNode)
    {
        if(!marked[startNode])
        {
            // create a new cluster
            clusters.emplace_back();
            Cluster & cluster = clusters.back();

            // insert the first node in the cluster
            cluster << startNode;
            marked[startNode] = true;

            // fill the connected components
            std::queue<int> q;
            q.push(startNode);
            while(!q.empty())
            {
                int node = q.front();
                q.pop();
                int nNeighbours = neighbours[node].size();
                for(int i=0; i<nNeighbours; ++i)
                {
                    int neighbour = neighbours[node][i];
                    if(!marked[neighbour])
                    {
                        marked[neighbour] = true;
                        q.push(neighbour);
                        cluster << neighbour;
                    }
                }
            }
        }
    }
    int nClusters= clusters.size();

#if MYDEBUG
    /////////  Print clusters  /////////
    std::cout << "Clustering:" << std::endl;
    std::cout << "  Self nodes indices = [ 0 .. " << splitNodes.nSelf-1 << " ]" << std::endl;
    std::cout << "  Existing nodes indices = [ ";
    if(nSplit-1 < splitNodes.nSelf)
        std::cout << "]" << std::endl;
    else if(nSplit-1 == splitNodes.nSelf)
        std::cout << splitNodes.nSelf << " ]" << std::endl;
    else
        std::cout << splitNodes.nSelf << " .. " << nSplit-1 << " ]" << std::endl;
    std::cout << "  Clusters: ";
    for(int k=0; k<nClusters; ++k)
    {
        std::cout << "[ ";
        int nConnected = clusters[k].size();
        for(int i=0; i<nConnected; ++i)
            std::cout << clusters[k].indices[i] << " ";
        std::cout << "] ";
    }
    std::cout << std::endl;
#endif


    // --------------------------------------------------------------------
    // ----------- Detect when the sketched edge must be a loop -----------
    // --------------------------------------------------------------------

    // TODO: Change the following code. Instead, we should detect when the first and last vertex of the sketched edge must be destroyed

    // I suppose no cluster has no self node in it. The reason is that if
    // an existing node is taken into account in this algo, then it has split
    // the sketched curve too, or will be use as start/end vertex.
    //
    // Edit: I think the above is wrong, since end points of intersected edges
    //       are automatically added as nodes
    //
    // Hence, all clusters have at least one self node. In the case of a loop,
    // this mean there is only one cluster, made of two self nodes.
    bool isClosed = false;
    int nSelf = splitNodes.nSelf;
    if(nSelf == 2     &&
            nClusters == 1 &&
            clusters[0].size() == 2 )
    {
        isClosed = true;
    }


    // --------------------------------------------------------------------
    // ---------------------- Process the clusters ------------------------
    // --------------------------------------------------------------------

    // From the indices in the cluster, compute:
    //   - how many of them are existing nodes
    //   - if zero: compute the mean, create new node
    //   - if one: basically do nothing
    //   - if more than one: for each index, compute closest existing node in cluster

    // Note: nSelfSplitValues = nSelfSplitNodes, by definition of nSelfSplitNodes
    // The actual nodes corresponding to the
    // self interections, already existing or newly created
    std::vector<KeyVertex*> selfNodes;
    selfNodes.resize(nSelf);

    if(isClosed)
    {
        // nothing to do
    }
    else
    {
        for(int i=0; i<nClusters; ++i)
        {
            Cluster & cluster = clusters[i];

            // How many existing nodes are in this cluster?
            // Who are they?
            //
            // todo? : what if no self nodes are in this cluster? Is this even possible? likely?
            //
            std::vector<KeyVertex *> existing;
            for(int i : cluster.indices)
                if(i>=nSelf)
                    existing << splitNodes.existingNodes[i-nSelf];

            // If none of them, compute the mean vertex
            // and create a new node
            if(existing.size() == 0)
            {
                // Create the new node
                KeyVertex * newNode = newKeyVertex(timeInteractivity_);

                // compute node position as mean of all self intersections
                Eigen::Vector2d mean(0,0);
                int n = cluster.size(); // we know size() > 0
                for(int i=0; i<n; ++i)
                {
                    int idx = cluster.indices[i];

                    // contributes to mean
                    mean += Eigen::Vector2d(splitNodes[idx].x(), splitNodes[idx].y());

                    // set the new node to be my node
                    selfNodes[idx] = newNode;
                }
                // Set node position
                mean /= (double) n;
                newNode->setPos(mean);
            }
            else if (existing.size() == 1)
            {
                for(int i : cluster.indices)
                    if(i<nSelf)
                        selfNodes[i] = existing.front();
            }
            else
            {
                // for now, just use first one instead of the closest one. Todo: improve this
                for(int i : cluster.indices)
                    if(i<nSelf)
                        selfNodes[i] = existing.front();

            }
        }
    }

    // --------------------------------------------------------------------
    // -------------------- Split the drawn curve -------------------------
    // --------------------------------------------------------------------

    std::vector<SketchedEdge,Eigen::aligned_allocator<SketchedEdge> > curves;
    if(!isClosed)
        curves = sketchedEdge_->curve().split(selfSplitValues);

    // Create topology and retarget drawn curve
    if(isClosed)
    {
        // retarget curve
        EdgeSample vStart = sketchedEdge_->curve().start();
        EdgeSample vEnd = sketchedEdge_->curve().end();
        double meanX = 0.5 * (vStart.x() + vEnd.x());
        double meanY = 0.5 * (vStart.y() + vEnd.y());
        vStart.setX(meanX);
        vStart.setY(meanY);
        vEnd.setX(meanX);
        vEnd.setY(meanY);
        sketchedEdge_->curve().setEndPoints(vStart, vEnd);

        // Create geometry out of it
        EdgeGeometry * geometry = new LinearSpline(sketchedEdge_->curve(), true);
        KeyEdge * iedge = newKeyEdge(timeInteractivity_, geometry);

        // if planar map mode, the loop can "cut" a face
        if(global()->planarMapMode())
        {
            if(hoveredFaceOnMousePress_)
            {
                // Cut face with loop
                KeyEdgeSet loopCycle;
                loopCycle << iedge;
                Cycle newCycle(loopCycle);
                hoveredFaceOnMousePress_->addCycle(newCycle);
                KeyFace * iFace = newKeyFace(loopCycle);
                iFace->setColor(hoveredFaceOnMousePress_->color());
                InbetweenFaceSet sfacesbefore = hoveredFaceOnMousePress_->temporalStarBefore();
                foreach(InbetweenFace * sface, sfacesbefore)
                    sface->addAfterFace(iFace);
                InbetweenFaceSet sfacesafter = hoveredFaceOnMousePress_->temporalStarAfter();
                foreach(InbetweenFace * sface, sfacesafter)
                    sface->addBeforeFace(iFace);
            }
        }
    }
    else
    {
        // if planar map mode, the first and last vertices can "cut" faces
        // by being added as Steiner cycles
        if(global()->planarMapMode() && nSelf>0)
        {
            KeyVertex * firstVertex = selfNodes[0];
            KeyVertex * lastVertex = selfNodes[nSelf-1];

            if(hoveredFaceOnMousePress_ &&
                    !(hoveredFaceOnMousePress_->spatialBoundary().contains(firstVertex)))
            {
                // Cut face with Steiner verte
                Cycle newCycle(firstVertex);
                hoveredFaceOnMousePress_->addCycle(newCycle);
            }
            if(hoveredFaceOnMouseRelease_ && (firstVertex != lastVertex) &&
                    !(hoveredFaceOnMouseRelease_->spatialBoundary().contains(lastVertex)))
            {
                // Cut face with Steiner vertex
                Cycle newCycle(lastVertex);
                hoveredFaceOnMouseRelease_->addCycle(newCycle);
            }
        }


        for(int i=0; i<nSelf-1; ++i)
        {
            // end nodes
            KeyVertex * startNode = selfNodes[i];
            KeyVertex * endNode = selfNodes[i+1];

            // retarget curve
            EdgeSample vStart = curves[i].start();
            vStart.setX(startNode->pos()[0]);
            vStart.setY(startNode->pos()[1]);
            EdgeSample vEnd = curves[i].end();
            vEnd.setX(endNode->pos()[0]);
            vEnd.setY(endNode->pos()[1]);
            curves[i].setEndPoints(vStart, vEnd);

            // Create geometry out of it
            EdgeGeometry * geometry = new LinearSpline(curves[i]);
            KeyEdge * iedge = 0;
            if(geometry->length() > tolerance)
                iedge = newKeyEdge(timeInteractivity_, startNode, endNode, geometry);

            // if planar map mode, cut a potential face underneath
            if(iedge && global()->planarMapMode())
            {
                // find a face to cut
                KeyFaceSet startFaces = startNode->spatialStar();
                KeyFaceSet endFaces = endNode->spatialStar();
                KeyFaceSet faces = startFaces;
                faces.intersect(endFaces);
                if(useFaceToConsiderForCutting)
                    faces.intersect(facesToConsiderForCutting_);

                if(!faces.isEmpty())
                {
                    // For now, just use the first face
                    KeyFace * face = *faces.begin();

                    // Cut the face by the new edge
                    CutFaceFeedback feedback;
                    cutFace_(face,iedge, &feedback);
                    if(useFaceToConsiderForCutting)
                    {
                        foreach(KeyFace * face, feedback.deletedFaces)
                            facesToConsiderForCutting_.remove(face);
                        foreach(KeyFace * face, feedback.newFaces)
                            facesToConsiderForCutting_.insert(face);
                    }
                }
            }
        }
    }
}

// --------------------- Sculpting ------------------------

void VAC::updateSculpt(double x, double y, Time time)
{
    double radius = global()->sculptRadius();
    timeInteractivity_ = time;
    KeyEdgeList iedges = instantEdges(timeInteractivity_);
    double minD = std::numeric_limits<double>::max();
    sculptedEdge_ = 0;
    foreach(KeyEdge * iedge, iedges)
    {
        double d = iedge->updateSculpt(x, y, radius);
        if(d<radius && d<minD)
        {
            minD = d;
            sculptedEdge_ = iedge;
        }
    }
}

void VAC::beginSculptDeform(double x, double y)
{
    if(sculptedEdge_)
    {
        sculptedEdge_->beginSculptDeform(x, y);
    }
}

void VAC::continueSculptDeform(double x, double y)
{
    if(sculptedEdge_)
    {
        sculptedEdge_->continueSculptDeform(x, y);
        //emit changed();
    }
}

void VAC::endSculptDeform()
{
    if(sculptedEdge_)
    {
        sculptedEdge_->endSculptDeform();
        //emit changed(); // done manually by View, after calling updatePicking(newX, newY)
        emit checkpoint();
    }
}

void VAC::beginSculptEdgeWidth(double x, double y)
{
    if(sculptedEdge_)
    {
        sculptedEdge_->beginSculptEdgeWidth(x, y);
    }
}

void VAC::continueSculptEdgeWidth(double x, double y)
{
    if(sculptedEdge_)
    {
        sculptedEdge_->continueSculptEdgeWidth(x, y);
        //emit changed();
    }
}

void VAC::endSculptEdgeWidth()
{
    if(sculptedEdge_)
    {
        sculptedEdge_->endSculptEdgeWidth();
        //emit changed(); // done manually by View, after calling updatePicking(newX, newY)
        emit checkpoint();
    }
}

void VAC::beginSculptSmooth(double /*x*/, double /*y*/)
{
    /* useless
        if(sculptedEdge_)
        {
            sculptedEdge_->beginSculptSmooth(x, y);
        }
        */
}

void VAC::continueSculptSmooth(double x, double y)
{
    updateSculpt(x, y, timeInteractivity_);
    if(sculptedEdge_)
    {
        // WARNING: sculptedEdge_ may have changed, and then sculptedEdge_->continueSculptSmooth(x, y);
        //          called without sculptedEdge_->beginSculptSmooth(x, y); called beforehand
        sculptedEdge_->continueSculptSmooth(x, y);
        //emit changed();
    }
}

void VAC::endSculptSmooth()
{
    if(sculptedEdge_)
    {
        sculptedEdge_->endSculptSmooth();
        //emit changed(); // done manually by View, after calling updatePicking(newX, newY)
        emit checkpoint();
    }
}


// ------------- User action: connect objects -------------



InbetweenVertex * VAC::inbetweenVertices_(KeyVertex * v1, KeyVertex * v2)
{
    InbetweenVertex * stv = newInbetweenVertex(v1, v2);
    stv->setColor(v1->color());
    return stv;
}

namespace
{

void findAnimatedVertexRec(KeyVertex * visitedVertex, KeyVertex * targetVertex, QMap<KeyVertex*,InbetweenVertex*> & next)
{
    // If already visited, do nothing
    if(!next.contains(visitedVertex))
    {
        if(visitedVertex->time() >= targetVertex->time())
        {
            // Terminal case 1: fail
            next[visitedVertex] = 0;
        }
        else
        {
            InbetweenVertexSet svs = visitedVertex->temporalStarAfter();
            foreach(InbetweenVertex * sv, svs)
            {
                KeyVertex * afterVertex = sv->afterVertex();
                if(afterVertex == targetVertex)
                {
                    // Terminal case 2: success
                    next[visitedVertex] = sv;
                    break;
                }
                else
                {
                    // Recursion
                    findAnimatedVertexRec(afterVertex, targetVertex,next);
                    if(next[afterVertex])
                    {
                        // Recursive case 1: success
                        next[visitedVertex] = sv;
                        break;
                    }
                }
            }

            // Recursive case 2: fail
            if(!next.contains(visitedVertex))
            {
                next[visitedVertex] = 0;
            }
        }
    }
}

AnimatedVertex findOrCreateAnimatedVertex(VAC * vac, KeyVertex * v1, KeyVertex * v2)
{
    assert(v1->time() != v2->time());

    if(v1->time() > v2->time())
        std::swap(v1,v2);

    QMap<KeyVertex*,InbetweenVertex*> next;
    findAnimatedVertexRec(v1,v2,next);

    InbetweenVertexList res;

    // Success case: get the animated vertex made of existing inbetween edges
    KeyVertex * v = v1;
    while( (v != v2) && (next[v]))
    {
        InbetweenVertex * sv = next[v];
        res << sv;
        v = sv->afterVertex();
    }

    // Fail case: create new inbetween edge
    if(res.isEmpty())
    {
        res << vac->newInbetweenVertex(v1, v2);
    }

    return AnimatedVertex(res);
}

}

InbetweenEdge * VAC::inbetweenEdges_(KeyEdge * e1, KeyEdge * e2)
{
    // closed edges
    if(e1->isClosed() && e2->isClosed())
    {
        KeyHalfedge h1(e1,true);
        KeyHalfedge h2(e2,haveSameOrientation(e1,e2));
        Cycle cycle1(QList<KeyHalfedge>() << h1);
        Cycle cycle2(QList<KeyHalfedge>() << h2);

        InbetweenEdge * ste = newInbetweenEdge(cycle1,cycle2);
        ste->setColor(e1->color());

        return ste;
    }
    // open edges
    else if(!e1->isClosed() && !e2->isClosed())
    {
        KeyHalfedge h1(e1,true);
        KeyHalfedge h2(e2,haveSameOrientation(e1,e2));
        KeyVertex * vstart1 = h1.startVertex();
        KeyVertex * vstart2 = h2.startVertex();
        KeyVertex * vend1 = h1.endVertex();
        KeyVertex * vend2 = h2.endVertex();
        Path path1(QList<KeyHalfedge>() << h1);
        Path path2(QList<KeyHalfedge>() << h2);
        AnimatedVertex avstart = findOrCreateAnimatedVertex(this,vstart1,vstart2);
        AnimatedVertex avend = findOrCreateAnimatedVertex(this,vend1,vend2);

        InbetweenEdge * ste = newInbetweenEdge(path1,path2,avstart,avend);
        ste->setColor(e1->color());

        return ste;
    }
    else // one closed edge and one open edge
    {
        qDebug("Operation aborted: you tried to inbetween a closed curve to an open path. This is not possible. Please split the closed curve before");
        return 0;
    }
}

void VAC::inbetweenSelection()
{
    // ---- get selected key cells ----

    KeyCellList list = selectedCells();

    // separate them into two lists with different times
    if(list.size() == 0 )
        return;
    Time t1 = list[0]->time();
    Time t2;
    bool ok = false;
    foreach(KeyCell * object, list)
    {
        if(!ok) // didn't find t2 yet
        {
            if(object->time() != t1) // found t2
            {
                t2 = object->time();
                ok = true;
            }
        }
        else // already found t2
        {
            if(object->time() != t1 && object->time() != t2) // found some t3
            {
                qDebug("Inbetweening: Selected objects span at least three different frames. Abort due to ambiguity.");
                return;
            }
        }
    }
    if(!ok)
    {
        qDebug("Inbetweening: Selected objects are all contained in the same frame, nothing to inbetween.");
        return;
    }
    if(t1>t2)
    {
        Time aux = t1;
        t1=t2;
        t2=aux;
    }
    KeyCellList list1;
    KeyCellList list2;
    foreach(KeyCell * object, list)
    {
        if(object->time() == t1)
            list1 << object;
        if(object->time() == t2)
            list2 << object;
    }
    // partition lists into vertices/edges/faces
    KeyVertexList vertices1 = list1;
    KeyVertexList vertices2 = list2;
    KeyEdgeList edges1 = list1;
    KeyEdgeList edges2 = list2;
    KeyFaceList faces1 = list1;
    KeyFaceList faces2 = list2;


    // ---------------- connect two key vertices --------------------

    if(list1.size() == 1 && list1[0]->toKeyVertex() &&
            list2.size() == 1 && list2[0]->toKeyVertex())
    {
        KeyVertex * v1 = list1[0]->toKeyVertex();
        KeyVertex * v2 = list2[0]->toKeyVertex();
        inbetweenVertices_(v1, v2);
    }


    // ---------------- connect two key edges --------------------

    else if(list1.size() == 1 && list1[0]->toKeyEdge() &&
            list2.size() == 1 && list2[0]->toKeyEdge())
    {
        KeyEdge * e1 = list1[0]->toKeyEdge();
        KeyEdge * e2 = list2[0]->toKeyEdge();
        inbetweenEdges_(e1,e2);
    }


    // ---------------- connect one key vertex to one key edge (Grow) --------------------

    else if(list1.size() == 1 && list1[0]->toKeyVertex() &&
            list2.size() == 1 && list2[0]->toKeyEdge())
    {
        KeyVertex * v1 = list1[0]->toKeyVertex();
        KeyEdge * e2 = list2[0]->toKeyEdge();
        // closed edge
        if(e2->isClosed())
        {
            KeyHalfedge h2(e2,true);
            Cycle cycle1(v1);
            Cycle cycle2(QList<KeyHalfedge>() << h2);
            newInbetweenEdge(cycle1,cycle2);
        }
        // open edges
        else
        {
            Path path1(v1);
            KeyHalfedge h2(e2,true);
            KeyVertex * vstart2 = h2.startVertex();
            KeyVertex * vend2 = h2.endVertex();
            Path path2(QList<KeyHalfedge>() << h2);
            AnimatedVertex avstart = findOrCreateAnimatedVertex(this,v1,vstart2);
            AnimatedVertex avend = findOrCreateAnimatedVertex(this,v1,vend2);
            InbetweenEdge * ste = newInbetweenEdge(path1,path2,avstart,avend);
            ste->setColor(e2->color());
        }
    }

    // ---------------- connect one key edge to one key vertex (Shrink) --------------------

    else if(list1.size() == 1 && list1[0]->toKeyEdge() &&
            list2.size() == 1 && list2[0]->toKeyVertex())
    {
        KeyEdge * e1 = list1[0]->toKeyEdge();
        KeyVertex * v2 = list2[0]->toKeyVertex();
        // closed edge
        if(e1->isClosed())
        {
            KeyHalfedge h1(e1,true);
            Cycle cycle1(QList<KeyHalfedge>() << h1);
            Cycle cycle2(v2);
            newInbetweenEdge(cycle1,cycle2);
        }
        // open edges
        else
        {
            Path path2(v2);
            KeyHalfedge h1(e1,true);
            KeyVertex * vstart1 = h1.startVertex();
            KeyVertex * vend1 = h1.endVertex();
            Path path1(QList<KeyHalfedge>() << h1);
            AnimatedVertex avstart = findOrCreateAnimatedVertex(this,vstart1,v2);
            AnimatedVertex avend = findOrCreateAnimatedVertex(this,vend1,v2);
            InbetweenEdge * ste = newInbetweenEdge(path1,path2,avstart,avend);
            ste->setColor(e1->color());
        }
    }

    // ---------------- General case: connect several edges to several edges --------------------

    else if(faces1.isEmpty() && faces2.isEmpty())
    {
        bool abort = false;

        // Try to convert selection at time t1 into a cycle or/and a path
        Cycle cycle1;
        Path path1;
        if(edges1.isEmpty())
        {
            if(vertices1.size() == 1)
            {
                cycle1 = Cycle(vertices1[0]);
                path1 = Path(vertices1[0]);
            }
            else
            {
                abort = true;
            }
        }
        else
        {
            KeyEdgeSet edges1Set = edges1;

            ProperCycle properCycle1(edges1Set);
            if(properCycle1.isValid())
            {
                cycle1 = Cycle(properCycle1);
                path1 = Path(properCycle1);
            }
            else
            {
                ProperPath properPath1(edges1Set);
                if(properPath1.isValid())
                    path1 = Path(properPath1);
            }
        }

        // Try to convert selection at time t2 into a cycle or/and a path
        Cycle cycle2;
        Path path2;
        if(edges2.isEmpty())
        {
            if(vertices2.size() == 1)
            {
                cycle2 = Cycle(vertices2[0]);
                path2 = Path(vertices2[0]);
            }
            else
            {
                abort = true;
            }
        }
        else
        {
            KeyEdgeSet edges2Set = edges2;

            ProperCycle properCycle2(edges2Set);
            if(properCycle2.isValid())
            {
                cycle2 = Cycle(properCycle2);
                path2 = Path(properCycle2);
            }
            else
            {
                ProperPath properPath2(edges2Set);
                if(properPath2.isValid())
                    path2 = Path(properPath2);
            }
        }

        // Decide if we should create a closed inbetween edge or an open inbetween edge,
        // with priority for closed inbetween edge, if possible. If none possible, abort
        // for now. In the future, use a smart optimization to optimize inbetweening for
        // arbitrary selection
        if(!abort)
        {
            if(cycle1.isValid() && cycle2.isValid())
            {
                // Choose cycle orientation (TODO: use a heuristic instead of settings checkbox)
                // TODO: choose cycle orientation, pick best offset
                if(DevSettings::getBool("inverse direction"))
                    cycle1 = cycle1.reversed();

                // Create closed inbetween edge
                newInbetweenEdge(cycle1,cycle2);


            }
            else if(path1.isValid() && path2.isValid())
            {
                // Choose cycle orientation (TODO: use a heuristic instead of settings checkbox)
                if(DevSettings::getBool("inverse direction"))
                    path1 = path1.reversed();

                // Create open inbetween edge
                KeyVertex * vstart1 = path1.startVertex();
                KeyVertex * vstart2 = path2.startVertex();
                KeyVertex * vend1 = path1.endVertex();
                KeyVertex * vend2 = path2.endVertex();
                InbetweenVertex * svstart = 0;
                InbetweenVertexSet vstartAfter = vstart1->temporalStarAfter();
                foreach(InbetweenVertex * sv, vstartAfter)
                    if(sv->afterVertex() == vstart2)
                        svstart = sv;
                if(!svstart)
                    svstart = newInbetweenVertex(vstart1, vstart2);
                InbetweenVertex * svend = 0;
                if(vstart1 == vend1 && vstart2 == vend2)
                    svend = svstart;
                if(!svend)
                {
                    InbetweenVertexSet vendAfter = vend1->temporalStarAfter();
                    foreach(InbetweenVertex * sv, vendAfter)
                        if(sv->afterVertex() == vend2)
                            svend = sv;
                }
                if(!svend)
                    svend = newInbetweenVertex(vend1, vend2);



                AnimatedVertex avstart(InbetweenVertexList() << svstart);
                AnimatedVertex avend(InbetweenVertexList() << svend);
                newInbetweenEdge(path1,path2,avstart,avend);

                // TODO: choose cycle orientation, pick best offset of one of the paths
                // is also a valid cycle.
            }
            else
            {
                abort = true;
            }
        }
    }

    deselectAll();
    emit needUpdatePicking();
    emit changed();
    emit checkpoint();
}

void VAC::keyframeSelection()
{
    keyframe_(selectedCells(), global()->activeTime());
    deselectAll();

    emit needUpdatePicking();
    emit changed();
    emit checkpoint();
}

class KeyframeHelper
{
public:
    KeyframeHelper(InbetweenCell * sc, VAC * vac) :
        wasHovered(false),
        wasSelected(false),
        vac(vac)
    {
        wasHovered = sc->isHovered();
        wasSelected = sc->isSelected();
    }

    void setKeyframe(KeyCell * kc)
    {
        if(kc)
        {
            if(wasHovered)
                vac->setHoveredCell(kc);
            if(wasSelected)
                vac->addToSelection(kc,false);
        }
    }

private:
    bool wasHovered;
    bool wasSelected;
    VAC * vac;
};

KeyCellSet VAC::keyframe_(const CellSet & cells, Time time)
{
    KeyCellSet keyframedCells;

    InbetweenCellSet inbetweenCells = cells;
    InbetweenCellSet relevantInbetweenCells;
    foreach(InbetweenCell * scell, inbetweenCells)
    {
        if(scell->exists(time))
            relevantInbetweenCells << scell;
    }

    InbetweenVertexSet inbetweenVertices = relevantInbetweenCells;
    InbetweenEdgeSet inbetweenEdges = relevantInbetweenCells;
    InbetweenFaceSet inbetweenFaces = relevantInbetweenCells;
    foreach(InbetweenVertex * svertex, inbetweenVertices)
    {
        KeyCell * keyframedCell = keyframe_(svertex,time);
        if(keyframedCell)
            keyframedCells << keyframedCell;
    }
    foreach(InbetweenEdge * sedge, inbetweenEdges)
    {
        KeyCell * keyframedCell = keyframe_(sedge,time);
        if(keyframedCell)
            keyframedCells << keyframedCell;
    }
    foreach(InbetweenFace * sface, inbetweenFaces)
    {
        KeyCell * keyframedCell = keyframe_(sface,time);
        if(keyframedCell)
            keyframedCells << keyframedCell;
    }

    return keyframedCells;
}

KeyVertex * VAC::keyframe_(InbetweenVertex * svertex, Time time)
{
    // Preprocess
    KeyframeHelper keyframHelper(svertex,this);

    // Create new cells
    KeyVertex * keyVertex = newKeyVertex(time, svertex->pos(time));
    InbetweenVertex * inbetweenVertexBefore = newInbetweenVertex(svertex->beforeVertex(),keyVertex);
    InbetweenVertex * inbetweenVertexAfter = newInbetweenVertex(keyVertex,svertex->afterVertex());

    // Transfer properties
    Color color = svertex->color();
    keyVertex->setColor(color);
    inbetweenVertexBefore->setColor(color);
    inbetweenVertexAfter->setColor(color);

    // Update incidence relationships
    InbetweenCellSet spatialStar = svertex->spatialStar();
    InbetweenEdgeSet inbetweenEdgesToUpdate = spatialStar;
    InbetweenFaceSet inbetweenFacesToUpdate = spatialStar;
    foreach(InbetweenEdge * sedge, inbetweenEdgesToUpdate)
    {
        assert(!sedge->isClosed());

        sedge->startAnimatedVertex_.replaceCells(svertex,inbetweenVertexBefore,inbetweenVertexAfter);
        sedge->endAnimatedVertex_.replaceCells(svertex,inbetweenVertexBefore,inbetweenVertexAfter);

        sedge->removeMeFromSpatialStarOf_(svertex);
        sedge->addMeToSpatialStarOf_(inbetweenVertexBefore);
        sedge->addMeToSpatialStarOf_(keyVertex);
        sedge->addMeToSpatialStarOf_(inbetweenVertexAfter);

        sedge->geometryChanged_();
    }
    foreach(InbetweenFace * sface, inbetweenFacesToUpdate)
    {
        for(int k=0; k < sface->cycles_.size(); ++k)
        {
            sface->cycles_[k].replaceInbetweenVertex(svertex,
                                                     inbetweenVertexBefore,keyVertex,inbetweenVertexAfter);
        }

        sface->removeMeFromSpatialStarOf_(svertex);
        sface->addMeToSpatialStarOf_(inbetweenVertexBefore);
        sface->addMeToSpatialStarOf_(keyVertex);
        sface->addMeToSpatialStarOf_(inbetweenVertexAfter);

        sface->geometryChanged_();
    }

    // Delete old cell
    deleteCell(svertex);

    // Postprocess
    keyframHelper.setKeyframe(keyVertex);

    // Return keyframe
    return keyVertex;
}


KeyEdge * VAC::keyframe_(InbetweenEdge * sedge, Time time)
{
    // Preprocess
    KeyframeHelper keyframHelper(sedge,this);

    // Keyframe boundary
    KeyVertex * startVertex = 0;
    KeyVertex * endVertex = 0;
    if(!sedge->isClosed())
    {
        VertexCellSet startVertices = sedge->startAnimatedVertex_.vertices();
        foreach(VertexCell * v, startVertices)
        {
            if(v->exists(time))
            {
                startVertex = v->toKeyVertex();
                if(!startVertex)
                {
                    startVertex = keyframe_(v->toInbetweenVertex(),time);
                }
                break;
            }
        }

        VertexCellSet endVertices = sedge->endAnimatedVertex_.vertices();
        foreach(VertexCell * v, endVertices)
        {
            if(v->exists(time))
            {
                endVertex = v->toKeyVertex();
                if(!endVertex)
                {
                    endVertex = keyframe_(v->toInbetweenVertex(),time);
                }
                break;
            }
        }
    }

    // Create new cells
    EdgeGeometry * geo = new LinearSpline(sedge->getSampling(time));
    KeyEdge * keyEdge = 0;
    InbetweenEdge * inbetweenEdgeBefore = 0;
    InbetweenEdge * inbetweenEdgeAfter = 0;
    if(sedge->isClosed())
    {
        // Create key cell
        keyEdge = newKeyEdge(time, geo);

        // Create boundary helpers
        KeyHalfedge halfedge(keyEdge,true);
        Cycle cycle(QList<KeyHalfedge>() << halfedge);

        // Create inbetween cells
        inbetweenEdgeBefore = newInbetweenEdge(sedge->beforeCycle(), cycle);
        inbetweenEdgeAfter = newInbetweenEdge(cycle, sedge->afterCycle());
    }
    else
    {
        // Create key cell
        keyEdge = newKeyEdge(time, startVertex, endVertex, geo);

        // Create boundary helpers
        AnimatedVertex startVertices = sedge->startAnimatedVertex_;
        InbetweenVertexList startVerticesBefore;
        InbetweenVertexList startVerticesAfter;
        for(int i=0; i<startVertices.size(); ++i)
        {
            InbetweenVertex * sv = startVertices[i];
            if(sv->beforeVertex()->time() < time)
                startVerticesBefore << sv;
            else
                startVerticesAfter << sv;
        }
        AnimatedVertex endVertices = sedge->endAnimatedVertex_;
        InbetweenVertexList endVerticesBefore;
        InbetweenVertexList endVerticesAfter;
        for(int i=0; i<endVertices.size(); ++i)
        {
            InbetweenVertex * sv = endVertices[i];
            if(sv->beforeVertex()->time() < time)
                endVerticesBefore << sv;
            else
                endVerticesAfter << sv;
        }
        KeyHalfedge halfedge(keyEdge,true);
        Path path(QList<KeyHalfedge>() << halfedge);

        // Create inbetween cells
        inbetweenEdgeBefore = newInbetweenEdge(sedge->beforePath(),path,
                                               AnimatedVertex(startVerticesBefore),
                                               AnimatedVertex(endVerticesBefore));
        inbetweenEdgeAfter = newInbetweenEdge(path,sedge->afterPath(),
                                              AnimatedVertex(startVerticesAfter),
                                              AnimatedVertex(endVerticesAfter));
    }


    // Update incidence relationships
    InbetweenCellSet spatialStar = sedge->spatialStar();
    InbetweenFaceSet inbetweenFacesToUpdate = spatialStar;
    foreach(InbetweenFace * sface, inbetweenFacesToUpdate)
    {

        for(int k=0; k < sface->cycles_.size(); ++k)
        {
            sface->cycles_[k].replaceInbetweenEdge(sedge,
                                                   inbetweenEdgeBefore,keyEdge,inbetweenEdgeAfter);
        }

        sface->removeMeFromSpatialStarOf_(sedge);
        sface->addMeToSpatialStarOf_(inbetweenEdgeBefore);
        sface->addMeToSpatialStarOf_(keyEdge);
        sface->addMeToSpatialStarOf_(inbetweenEdgeAfter);

        sface->geometryChanged_();
    }

    // Transfer properties
    Color color = sedge->color();
    keyEdge->setColor(color);
    inbetweenEdgeBefore->setColor(color);
    inbetweenEdgeAfter->setColor(color);

    // Set correct Z-ordering
    zOrdering_.moveBelowBoundary(keyEdge);
    zOrdering_.moveBelowBoundary(inbetweenEdgeBefore);
    zOrdering_.moveBelowBoundary(inbetweenEdgeAfter);

    // Delete old cell
    deleteCell(sedge);

    // Postprocess
    keyframHelper.setKeyframe(keyEdge);

    // Return keyframe
    return keyEdge;
}

KeyFace * VAC::keyframe_(InbetweenFace * sface, Time time)
{
    // Preprocess
    KeyframeHelper keyframHelper(sface,this);

    // Keyframe boundary
    for(int i=0; i<sface->numAnimatedCycles(); ++i)
    {
        // Keyframes inbetween vertices in cycle
        InbetweenVertexSet sverticesInCycle = sface->animatedCycle(i).cells();
        foreach(InbetweenVertex * svertex, sverticesInCycle)
            if(svertex->exists(time))
                keyframe_(svertex, time);

        // Keyframes inbetween edges in cycle
        InbetweenEdgeSet sedgesInCycle = sface->animatedCycle(i).cells();
        foreach(InbetweenEdge * sedge, sedgesInCycle)
            if(sedge->exists(time))
                keyframe_(sedge, time);
    }

    // Create key cell
    KeyFace * keyFace = newKeyFace(time);
    InbetweenFace * inbetweenFaceBefore = newInbetweenFace(QList<AnimatedCycle>(),
                                                           QSet<KeyFace*>(),
                                                           QSet<KeyFace*>());
    InbetweenFace * inbetweenFaceAfter = newInbetweenFace(QList<AnimatedCycle>(),
                                                          QSet<KeyFace*>(),
                                                          QSet<KeyFace*>());

    // "Split" each animated cycle into two animated cycle and one key cycle
    // Add these cycles to the boundary of new cells
    for(int i=0; i<sface->numAnimatedCycles(); ++i)
    {
        // --- Create before and after animated cycles ---

        // Create copy of cycles
        AnimatedCycle beforeCycle = sface->animatedCycle(i);
        AnimatedCycle afterCycle = sface->animatedCycle(i);

        // Compute "first" nodes
        AnimatedCycleNode * beforeCycleFirst = beforeCycle.first();
        AnimatedCycleNode * afterCycleFirst = afterCycle.getNode(time)->after();

        // Compute all nodes to delete
        QSet<AnimatedCycleNode*> beforeCycleNodesToDelete;
        foreach(AnimatedCycleNode * node, beforeCycle.nodes())
            if(!node->cell()->isBefore(time))
                beforeCycleNodesToDelete << node;
        QSet<AnimatedCycleNode*> afterCycleNodesToDelete;
        foreach(AnimatedCycleNode * node, afterCycle.nodes())
            if(!node->cell()->isAfter(time))
                afterCycleNodesToDelete << node;

        // Set pointers to deleted nodes to null instead
        foreach(AnimatedCycleNode * node, beforeCycle.nodes())
            if(beforeCycleNodesToDelete.contains(node->after()))
                node->setAfter(0);
        foreach(AnimatedCycleNode * node, afterCycle.nodes())
            if(afterCycleNodesToDelete.contains(node->before()))
                node->setBefore(0);

        // Set "first"
        beforeCycle.setFirst(beforeCycleFirst);
        afterCycle.setFirst(afterCycleFirst);

        // Delete nodes to delete
        foreach(AnimatedCycleNode * node, beforeCycleNodesToDelete)
            delete node;
        foreach(AnimatedCycleNode * node, afterCycleNodesToDelete)
            delete node;

        // Add cycles to new inbetween faces
        inbetweenFaceBefore->addAnimatedCycle(beforeCycle);
        inbetweenFaceAfter->addAnimatedCycle(afterCycle);

        // --- Create key cycle ---

        // Create copy of cycles
        AnimatedCycle animatedCycle = sface->animatedCycle(i);

        // Get node at time t.
        // It's guaranteed to points to a key cell, to have n->next(t) = n->next().
        // recursively, n->next() is guaranteed to have the same properties
        AnimatedCycleNode * firstNodeOfKeyCycle = animatedCycle.getNode(time);

        Cycle keyCycle;
        AnimatedCycleNode::CycleType keyCycleType = firstNodeOfKeyCycle->cycleType(time);
        if(keyCycleType == AnimatedCycleNode::SteinerCycle)
        {
            keyCycle = Cycle(firstNodeOfKeyCycle->cell()->toKeyVertex());
        }
        else if(keyCycleType == AnimatedCycleNode::SimpleCycle)
        {
            // Get closed halfedge
            KeyHalfedge halfedge(firstNodeOfKeyCycle->cell()->toKeyEdge(),firstNodeOfKeyCycle->side());

            // Compute how many time it's repeated
            int n = 1;
            AnimatedCycleNode * node = firstNodeOfKeyCycle->next();
            while(node != firstNodeOfKeyCycle)
            {
                node = node->next();
                ++n;
            }

            // Make cycle
            QList<KeyHalfedge> halfedgeList;
            for(int i=0; i<n; ++i)
                halfedgeList << halfedge;
            keyCycle = Cycle(halfedgeList);
        }
        else if(keyCycleType == AnimatedCycleNode::NonSimpleCycle)
        {
            // node->next() alternatively points to key vertices and key edges
            // we are only interested int the key edges.
            AnimatedCycleNode * firstEdgeNode = firstNodeOfKeyCycle;
            if(firstEdgeNode->cell()->toKeyVertex())
                firstEdgeNode = firstEdgeNode->next();

            // Make cycle
            AnimatedCycleNode * node = firstEdgeNode;
            QList<KeyHalfedge> halfedgeList;
            do
            {
                halfedgeList << KeyHalfedge(node->cell()->toKeyEdge(),node->side());
                node = node->next()->next();
            }
            while(node != firstEdgeNode);
            keyCycle = Cycle(halfedgeList);
        }

        keyFace->addCycle(keyCycle);
    }

    // Set temporal boundary
    inbetweenFaceBefore->setBeforeFaces(sface->beforeFaces());
    inbetweenFaceBefore->addAfterFace(keyFace);
    inbetweenFaceAfter->addBeforeFace(keyFace);
    inbetweenFaceAfter->setAfterFaces(sface->afterFaces());

    // Transfer properties
    Color color = sface->color();
    keyFace->setColor(color);
    inbetweenFaceBefore->setColor(color);
    inbetweenFaceAfter->setColor(color);

    // Set correct Z-ordering
    zOrdering_.moveBelowBoundary(keyFace);
    zOrdering_.moveBelowBoundary(inbetweenFaceBefore);
    zOrdering_.moveBelowBoundary(inbetweenFaceAfter);

    // Delete old cell
    deleteCell(sface);

    // Postprocess
    keyframHelper.setKeyframe(keyFace);

    // Return keyframe
    return keyFace;
}



QList<Cycle> VAC::createFace_computeCycles()
{
    // Create all cycles
    QList<Cycle> cycles;

    // Edges to use as non-Steiner cycles
    KeyEdgeSet edgeSet = selectedCells();
    SmartKeyEdgeSet smartKeyEdgeSet(edgeSet);
    for(int i=0; i<smartKeyEdgeSet.numConnectedComponents(); ++i)
    {
        SmartConnectedKeyEdgeSet & potentialCycle = smartKeyEdgeSet[i];
        if(potentialCycle.type() == SmartConnectedKeyEdgeSet::GENERAL )
        {
            global()->mainWindow()->statusBar()->showMessage(tr("Some selected edges were ambiguous and have been ignored"));
        }
        else if(potentialCycle.type() == SmartConnectedKeyEdgeSet::CLOSED_EDGE )
        {
            Cycle cycle(potentialCycle.edgeSet());
            if(cycle.isValid())
                cycles << cycle;
            else
            {
                //qDebug() << "Warning: invalid cycle while it is apparently a closed edge";
            }
        }
        else if(potentialCycle.type() == SmartConnectedKeyEdgeSet::OPEN_EDGE_LOOP )
        {
            Cycle cycle(potentialCycle.edgeSet());
            if(cycle.isValid())
                cycles << cycle;
            else
            {
                //qDebug() << "Warning: invalid cycle while it is apparently a looping open edge";
            }
        }
        else if(potentialCycle.type() == SmartConnectedKeyEdgeSet::SIMPLE_LOOP )
        {
            Cycle cycle(potentialCycle.edgeSet());
            if(cycle.isValid())
                cycles << cycle;
            else
            {
                //qDebug() << "Warning: invalid cycle while it is apparently a simple loop";
            }
        }
        else if(potentialCycle.type() == SmartConnectedKeyEdgeSet::OPEN_EDGE_PATH )
        {
            // Get edge
            KeyEdge * edge = potentialCycle.edge();

            // Create invisible edge
            KeyEdge * newEdge = newKeyEdge(edge->time(), edge->startVertex(), edge->endVertex());

            // Add it to cycle
            KeyEdgeSet newEdgeSet = potentialCycle.edgeSet();
            newEdgeSet << newEdge;
            Cycle cycle(newEdgeSet);
            if(cycle.isValid())
            {
                cycles << cycle;
            }
            else
            {
                deleteCell(newEdge);
                //qDebug() << "Warning: invalid cycle while it is apparently a "
                //            "non-looping open edge to which we added an invisible edge";
            }
        }
        else if(potentialCycle.type() == SmartConnectedKeyEdgeSet::SIMPLE_PATH )
        {
            // Get edge
            ProperPath path = potentialCycle.path();

            // Create invisible edge
            KeyEdge * newEdge = newKeyEdge(path.time(), path[0].startVertex(), path[path.size()-1].endVertex());

            // Add it to cycle
            KeyEdgeSet newEdgeSet = potentialCycle.edgeSet();
            newEdgeSet << newEdge;
            Cycle cycle(newEdgeSet);
            if(cycle.isValid())
            {
                cycles << cycle;
            }
            else
            {
                deleteCell(newEdge);
                //qDebug() << "Warning: invalid cycle while it is apparently a "
                //            "non-looping open edge to which we added an invisible edge";
            }
        }
        else if(potentialCycle.type() == SmartConnectedKeyEdgeSet::PATH_LOOP_DECOMPOSITION )
        {
            // Get edge
            CycleHelper hole = potentialCycle.hole();

            // --- Naive version for now, to be improved later ---

            // Create one cycle per loop
            for(int j=0; j<hole.nLoops(); ++j)
            {
                ProperCycle loop = hole.loop(j);
                KeyEdgeSet newEdgeSet;
                for(int k=0; k<loop.size(); ++k)
                    newEdgeSet << loop[k].edge;

                // Add it to cycle
                Cycle cycle(newEdgeSet);
                if(cycle.isValid())
                {
                    cycles << cycle;
                }
                else
                {
                    //qDebug() << "Warning: invalid cycle while it is made from a loop";
                }
            }
        }
    }

    // vertices to use as Steiner cycles
    KeyVertexSet vertexSet = selectedCells();
    KeyVertexSet verticesInClosureOfEdges = Algorithms::closure(edgeSet);
    vertexSet.subtract(verticesInClosureOfEdges);

    // Create steiner cycles
    foreach(KeyVertex * v, vertexSet)
    {
        Cycle cycle(v);
        if(cycle.isValid())
            cycles << cycle;
    }

    return cycles;
}

void VAC::createFace()
{
    // Compute cycles
    QList<Cycle> cycles = createFace_computeCycles();

    // Create face
    if(cycles.size() == 0)
    {
        QMessageBox::information(0, QObject::tr("operation aborted"),
                                 QObject::tr("Could not create a valid face from the selection"));
    }
    else
    {
        newKeyFace(cycles);

        emit needUpdatePicking();
        emit changed();
        emit checkpoint();
    }
}

void VAC::addCyclesToFace()
{
    // Compute cycles
    QList<Cycle> cycles = createFace_computeCycles();

    // The faces to which we should add the cycles
    KeyFaceSet faceSet = selectedCells();
    if(faceSet.size() == 0)
    {
        QMessageBox::information(0, QObject::tr("operation aborted"),
                                 QObject::tr("You need to select at least one face"));
        return;
    }

    // Add cycles to faces
    if(cycles.size() == 0)
    {
        QMessageBox::information(0, QObject::tr("operation aborted"),
                                 QObject::tr("Could not create a valid cycle from selection"));
    }
    else
    {
        foreach(KeyFace * face, faceSet)
            face->addCycles(cycles);

        emit needUpdatePicking();
        emit changed();
        emit checkpoint();
    }
}

void VAC::removeCyclesFromFace()
{
    // The faces to which we should add the cycles
    KeyFaceSet faceSet = selectedCells();
    if(faceSet.size() == 0)
    {
        QMessageBox::information(0, QObject::tr("operation aborted"),
                                 QObject::tr("You need to select at least one face"));
        return;
    }

    foreach(KeyFace * face, faceSet)
    {
        QList<Cycle> newCycles;
        for(int i=0; i<face->cycles_.size(); ++i)
        {
            bool keepCycle = true;
            foreach(KeyCell * cell, face->cycles_[i].cells())
            {
                if(cell->isSelected())
                {
                    keepCycle = false;
                    break;
                }
            }

            if(keepCycle)
                newCycles << face->cycles_[i];
        }

        if(newCycles.size() > 0)
        {
            face->setCycles(newCycles);
        }
        else
        {
            QMessageBox::information(0, QObject::tr("operation aborted"),
                                     QObject::tr("At least one cycle of the face "
                                                 "must be preserved"));
        }
    }

    emit needUpdatePicking();
    emit changed();
    emit checkpoint();

}

void VAC::changeColor()
{
    if(numSelectedCells() > 0)
    {
        QColor initialColor = (*selectedCells().begin())->color();
        QColor color = QColorDialog::getColor(initialColor, 0, tr("select the color for the selected cells"), QColorDialog::ShowAlphaChannel);

        if (color.isValid()) {
            foreach(Cell * cell, selectedCells())
            {
                cell->setColor(color);
            }
        }

        //emit needUpdatePicking();
        emit changed();
        emit checkpoint();
    }
}

void VAC::raise()
{
    if(numSelectedCells() > 0)
    {
        zOrdering_.raise(selectedCells());

        emit needUpdatePicking();
        emit changed();
        emit checkpoint();
    }
}

void VAC::lower()
{
    if(numSelectedCells() > 0)
    {
        zOrdering_.lower(selectedCells());

        emit needUpdatePicking();
        emit changed();
        emit checkpoint();
    }
}

void VAC::raiseToTop()
{
    if(numSelectedCells() > 0)
    {
        zOrdering_.raiseToTop(selectedCells());

        emit needUpdatePicking();
        emit changed();
        emit checkpoint();
    }
}

void VAC::lowerToBottom()
{
    if(numSelectedCells() > 0)
    {
        zOrdering_.lowerToBottom(selectedCells());

        emit needUpdatePicking();
        emit changed();
        emit checkpoint();
    }
}


void VAC::altRaise()
{
    if(numSelectedCells() > 0)
    {
        zOrdering_.altRaise(selectedCells());

        emit needUpdatePicking();
        emit changed();
        emit checkpoint();
    }
}

void VAC::altLower()
{
    if(numSelectedCells() > 0)
    {
        zOrdering_.altLower(selectedCells());

        emit needUpdatePicking();
        emit changed();
        emit checkpoint();
    }
}

void VAC::altRaiseToTop()
{
    if(numSelectedCells() > 0)
    {
        zOrdering_.altRaiseToTop(selectedCells());

        emit needUpdatePicking();
        emit changed();
        emit checkpoint();
    }
}

void VAC::altLowerToBottom()
{
    if(numSelectedCells() > 0)
    {
        zOrdering_.altLowerToBottom(selectedCells());

        emit needUpdatePicking();
        emit changed();
        emit checkpoint();
    }
}

void VAC::changeEdgeWidth()
{
    KeyEdgeSet iedges = selectedCells();
    if(iedges.size() > 0)
    {
        bool ok;
        int i = QInputDialog::getInt(0, tr("select new edge width"),
                                     tr("width:"), 10, 0, 100, 1, &ok);
        if (ok)
        {
            foreach(KeyEdge * iedge, iedges)
            {
                iedge->setWidth((double) i);
            }

        }

        emit needUpdatePicking();
        emit changed();
        emit checkpoint();
    }
}


void VAC::glue()
{
    KeyVertexSet vertexSet = selectedCells();
    KeyEdgeSet edgeSet = selectedCells();

    if(edgeSet.size() == 2)
    {
        KeyEdgeList e = edgeSet.toList();
        glue_(e[0],e[1]);
    }
    else if (vertexSet.size() == 2)
    {
        KeyVertexList v = vertexSet.toList();
        glue_(v[0],v[1]);
    }
    else
    {
        QMessageBox::information(0, QObject::tr("Glue: operation aborted"),
                                 QObject::tr("Please select either two endpoints or two curves prior to trigger this action."));
        return;
    }

    emit needUpdatePicking();
    emit changed();
    emit checkpoint();
}

void VAC::unglue()
{
    KeyVertexSet vertexSet = selectedCells();
    KeyEdgeSet edgeSet = selectedCells();

    foreach(KeyEdge * iedge, edgeSet)
        unglue_(iedge);
    foreach(KeyVertex * ivertex, vertexSet)
        unglue_(ivertex);

    emit needUpdatePicking();
    emit changed();
    emit checkpoint();
}

void VAC::uncut()
{
    KeyVertexSet vertexSet = selectedCells();
    KeyEdgeSet edgeSet = selectedCells();

    bool hasbeenCut = false;

    foreach(KeyEdge * iedge, edgeSet)
        hasbeenCut |= uncut_(iedge);
    foreach(KeyVertex * ivertex, vertexSet)
        hasbeenCut |= uncut_(ivertex);

    if(hasbeenCut)
    {
        deselectAll();

        emit needUpdatePicking();
        emit changed();
        emit checkpoint();
    }
}

void VAC::cut(VAC* & clipboard)
{
    timeCopy_ = global()->activeTime();

    if(selectedCells().isEmpty())
        return;

    if(clipboard)
        delete clipboard;

    clipboard = subcomplex(selectedCells());
    smartDelete_(selectedCells());

    emit needUpdatePicking();
    emit changed();
    emit checkpoint();
}

void VAC::copy(VAC* & clipboard)
{
    timeCopy_ = global()->activeTime();

    if(selectedCells().isEmpty())
        return;

    if(clipboard)
        delete clipboard;

    clipboard = subcomplex(selectedCells());
}

void VAC::paste(VAC *& clipboard)
{
    if(!clipboard) return;

    // Get different between current time and copy time
    Time deltaTime = global()->activeTime() - timeCopy_;

    // Offset clipboard VAC by deltaTime
    VAC * cloneOfClipboard = clipboard->clone();
    KeyCellSet keyCells = cloneOfClipboard->cells();
    foreach(KeyCell * kc, keyCells)
    {
        kc->time_ = kc->time_ + deltaTime;
    }

    // Import into this VAC and set as selection
    removeFromSelection(selectedCells());
    import(cloneOfClipboard, true);

    // Delete clone
    delete cloneOfClipboard;

    emit needUpdatePicking();
    emit changed();
    emit checkpoint();
}

void VAC::motionPaste(VAC* & clipboard)
{
    if(!clipboard) return;

    // Check that it is possible to motion paste
    {
        InbetweenCellSet inbetweenCells = clipboard->cells();
        if(!inbetweenCells.isEmpty())
        {
            QMessageBox::information(0, QObject::tr("operation aborted"),
                                     QObject::tr("Cannot motion-paste: the clipboard contains inbetween cells."));
            return;
        }

        // Other checks should be done. For now, just assume it's possible, crash otherwise.
    }

    // Get different between current time and copy time
    Time deltaTime = global()->activeTime() - timeCopy_;
    if(deltaTime.frame() == 0)
    {
        QMessageBox::information(0, QObject::tr("operation aborted"),
                                 QObject::tr("Cannot motion-paste: the frame where you motion-paste must be different from the frame you copy."));
        return;
    }

    // Offset clipboard VAC by deltaTime
    VAC * cloneOfClipboard = clipboard->clone();
    KeyCellSet keyCells = cloneOfClipboard->cells();
    foreach(KeyCell * kc, keyCells)
    {
        kc->time_ = kc->time_ + deltaTime;
    }

    // Import into this VAC and set as selection
    removeFromSelection(selectedCells());
    QMap<int,int> idMap = import(cloneOfClipboard, true);

    // Separate vertices/edges/faces into different maps
    QMap<KeyVertex *, KeyVertex * > v1ToV2;
    QMap<KeyEdge *, KeyEdge * > e1ToE2;
    QMap<KeyFace *, KeyFace * > f1ToF2;
    QMapIterator<int, int> i(idMap);
    while (i.hasNext())
    {
        i.next();

        int copyID =  (deltaTime.frame() > 0) ? i.key()   : i.value();
        int pasteID = (deltaTime.frame() > 0) ? i.value() : i.key();
        Cell * copyCell = getCell(copyID);
        Cell * pasteCell = getCell(pasteID);
        KeyVertex * v1 = copyCell  ? copyCell->toKeyVertex()  : 0;
        KeyVertex * v2 = pasteCell ? pasteCell->toKeyVertex() : 0;
        KeyEdge * e1 = copyCell  ? copyCell->toKeyEdge()  : 0;
        KeyEdge * e2 = pasteCell ? pasteCell->toKeyEdge() : 0;
        KeyFace * f1 = copyCell  ? copyCell->toKeyFace()  : 0;
        KeyFace * f2 = pasteCell ? pasteCell->toKeyFace() : 0;
        if(v1 && v2)
            v1ToV2[v1] = v2;
        else if(e1 && e2)
            e1ToE2[e1] = e2;
        else if(f1 && f2)
            f1ToF2[f1] = f2;
    }

    // Create Inbetween vertices
    QMap<KeyVertex *, InbetweenVertex* > v1ToSTV;
    QMapIterator<KeyVertex *, KeyVertex * > iv(v1ToV2);
    while (iv.hasNext())
    {
        iv.next();
        KeyVertex * v1 = iv.key();
        KeyVertex * v2 = iv.value();
        InbetweenVertex * stv = inbetweenVertices_(v1,v2);
        assert(stv);

        v1ToSTV[v1] = stv;
    }

    // Create Inbetween edges
    QMap<KeyEdge *, InbetweenEdge* > e1ToSTE;
    QMapIterator<KeyEdge *, KeyEdge * > ie(e1ToE2);
    while (ie.hasNext())
    {
        ie.next();
        KeyEdge * e1 = ie.key();
        KeyEdge * e2 = ie.value();
        InbetweenEdge * ste = inbetweenEdges_(e1,e2);
        assert(ste);

        e1ToSTE[e1] = ste;
    }

    // Create Inbetween faces
    QMapIterator<KeyFace *, KeyFace * > iF(f1ToF2);
    while (iF.hasNext())
    {
        iF.next();
        KeyFace * f1 = iF.key();
        KeyFace * f2 = iF.value();

        // Some safety checks
        assert(f1->cycles_.size() == f2->cycles_.size());
        int nCycles = f1->cycles_.size();
        for(int k = 0; k<nCycles; ++k)
        {
            assert(f1->cycles_[k].halfedges_.size() == f2->cycles_[k].halfedges_.size());
        }

        // Create the inbetween face
        QSet<KeyFace*> beforeFaces; beforeFaces << f1;
        QSet<KeyFace*> afterFaces; afterFaces << f2;
        QList<AnimatedCycle> cycles;
        for(int k = 0; k<nCycles; ++k)
        {
            QList<AnimatedCycleNode *> nodes;
            AnimatedCycle cycle;
            int nHalfedges = f1->cycles_[k].halfedges_.size();

            // Create nodes. Set cell and side. Set before=after=NULL.
            for(int i=0; i<nHalfedges; i++)
            {
                KeyHalfedge h = f1->cycles_[k].halfedges_[i];
                KeyEdge * e1 = h.edge;
                InbetweenEdge * ste = e1ToSTE[e1];

                AnimatedCycleNode * edgeNode = new AnimatedCycleNode(ste);
                nodes << edgeNode;
                edgeNode->setSide(h.side);
                edgeNode->setBefore(0);
                edgeNode->setAfter(0);

                if(!e1->isClosed())
                {
                    KeyVertex * v1 = h.endVertex();
                    InbetweenVertex * stv = v1ToSTV[v1];

                    AnimatedCycleNode * vertexNode = new AnimatedCycleNode(stv);
                    nodes << vertexNode;
                    vertexNode->setBefore(0);
                    vertexNode->setAfter(0);
                }
            }

            // Special case of Steiner Vertex
            if(f1->cycles_[k].vertex_)
            {
                assert(nodes.isEmpty());

                KeyVertex * v1 = f1->cycles_[k].vertex_;
                InbetweenVertex * stv = v1ToSTV[v1];

                AnimatedCycleNode * vertexNode = new AnimatedCycleNode(stv);
                nodes << vertexNode;
                vertexNode->setBefore(0);
                vertexNode->setAfter(0);

            }

            // Set previous and next
            int nNodes = nodes.size();
            for(int i=1; i<=nNodes; i++) // Caution: (-1 % n) == -1
            {
                nodes[i % nNodes]->setPrevious( nodes[ (i-1) % nNodes ] );
                nodes[i % nNodes]->setNext( nodes[ (i+1) % nNodes ] );
            }

            // Create animated cycle
            assert(!nodes.isEmpty());
            cycles << AnimatedCycle(nodes.first());
        }

        // Create inbetween face
        InbetweenFace * stf = newInbetweenFace(cycles, beforeFaces, afterFaces);
        stf->setColor(f1->color());
    }

    // Delete clone
    delete cloneOfClipboard;

    informTimelineOfSelection();
    emit needUpdatePicking();
    emit changed();
    emit checkpoint();

}

void VAC::resetCellsToConsiderForCutting()
{
    facesToConsiderForCutting_.clear();
    edgesToConsiderForCutting_.clear();
}

void VAC::updateCellsToConsiderForCutting()
{
    if(hoveredCell_)
    {
        KeyFace * iface = hoveredCell_->toKeyFace();
        KeyEdge* iedge= hoveredCell_->toKeyEdge();
        if(iface)
            facesToConsiderForCutting_.insert(iface);
        if(iedge)
            edgesToConsiderForCutting_.insert(iedge);
    }
}

// ----- Selection -----

void VAC::setHoveredCell(Cell * cell)
{
    setNoHoveredCell();

    if(cell)
    {
        hoveredCell_ = cell;
        hoveredCell_->setHovered(true);
    }
}

void VAC::setNoHoveredCell()
{
    if(hoveredCell_)
    {
        hoveredCell_->setHovered(false);
        hoveredCell_ = 0;
    }
}

void VAC::informTimelineOfSelection()
{
    int selectionType = 0;
    double t = 0;
    double t1 = 0;
    double t2 = 0;

    foreach(Cell * cell, selectedCells_)
    {
        KeyCell * keyCell = cell->toKeyCell();
        InbetweenCell * inbetweenCell = cell->toInbetweenCell();
        if(keyCell)
        {
            if(selectionType != 1)
            {
                selectionType = 1;
                t = keyCell->time().floatTime();
                t1 = std::numeric_limits<double>::lowest(); // = -max
                t2 = std::numeric_limits<double>::max();
            }

            InbetweenCellSet beforeCells = keyCell->temporalStarBefore();
            foreach(InbetweenCell * scell, beforeCells)
            {
                double tbefore = scell->beforeTime().floatTime();
                t1 = std::max(tbefore,t1);
            }

            InbetweenCellSet afterCells = keyCell->temporalStarAfter();
            foreach(InbetweenCell * scell, afterCells)
            {
                double tafter = scell->afterTime().floatTime();
                t2 = std::min(tafter,t2);
            }
        }
        else if(inbetweenCell && (selectionType!=1) )
        {
            selectionType = 2;
            t1 = inbetweenCell->beforeTime().floatTime();
            t2 = inbetweenCell->afterTime().floatTime();
            break; // i.e., ignore further cells
        }
    }

    if(selectionType == 1)
    {
        if(t1 == std::numeric_limits<double>::lowest())
            t1 = t;
        if(t2 == std::numeric_limits<double>::max())
            t2 = t;
    }

    Timeline * timeline = global()->timeline();
    timeline->setSelectionType(selectionType);
    timeline->setT(t);
    timeline->setT1(t1);
    timeline->setT2(t2);
}

void VAC::addToSelection(Cell * cell, bool emitSignal)
{
    if(cell && !cell->isSelected())
    {
        selectedCells_ << cell;
        cell->setSelected(true);
        emitSelectionChanged_();
        if(emitSignal)
        {
            emit changed();
        }
    }
}

void VAC::removeFromSelection(Cell * cell, bool emitSignal)
{
    if(cell && cell->isSelected())
    {
        selectedCells_.remove(cell);
        cell->setSelected(false);
        emitSelectionChanged_();
        if(emitSignal)
        {
            emit changed();
        }
    }
}

void VAC::toggleSelection(Cell * cell, bool emitSignal)
{
    if(cell)
    {
        if(cell->isSelected())
            removeFromSelection(cell,emitSignal);
        else
            addToSelection(cell,emitSignal);
    }
}

void VAC::addToSelection(const CellSet & cells, bool emitSignal)
{
    beginAggregateSignals_();
    foreach(Cell * c, cells)
        addToSelection(c, false);
    endAggregateSignals_();

    if(emitSignal)
    {
        emit changed();
    }
}

void VAC::removeFromSelection(const CellSet & cells, bool emitSignal)
{
    beginAggregateSignals_();
    foreach(Cell * c, cells)
        removeFromSelection(c, false);
    endAggregateSignals_();

    if(emitSignal)
    {
        emit changed();
    }
}


void VAC::toggleSelection(const CellSet & cells, bool emitSignal)
{
    beginAggregateSignals_();
    foreach(Cell * c, cells)
        toggleSelection(c, false);
    endAggregateSignals_();

    if(emitSignal)
    {
        emit changed();
    }
}

void VAC::setSelectedCell(Cell * cell, bool emitSignal)
{
    if(cell)
    {
        CellSet cells;
        cells.insert(cell);
        setSelectedCells(cells, emitSignal);
    }
}

void VAC::setSelectedCells(const CellSet & cells, bool emitSignal)
{
    foreach(Cell * cell, selectedCells_)
        cell->setSelected(false);

    foreach(Cell * cell, cells)
        cell->setSelected(true);

    selectedCells_ = cells;

    emitSelectionChanged_();
    if(emitSignal)
    {
        emit changed();
    }
}

void VAC::selectAll(bool emitSignal)
{
    addToSelection(cells(), emitSignal);
}

void VAC::selectConnected(bool emitSignal)
{
    addToSelection(Algorithms::connected(selectedCells()), emitSignal);
}

void VAC::selectClosure(bool emitSignal)
{
    addToSelection(Algorithms::closure(selectedCells()), emitSignal);
}

void VAC::selectVertices(bool emitSignal)
{
    CellSet cellsToSelect;
    foreach(Cell * c, selectedCells())
        if(c->toKeyVertex())
            cellsToSelect.insert(c);

    setSelectedCells(cellsToSelect,emitSignal);
}

void VAC::selectEdges(bool emitSignal)
{
    CellSet cellsToSelect;
    foreach(Cell * c, selectedCells())
        if(c->toKeyEdge())
            cellsToSelect.insert(c);

    setSelectedCells(cellsToSelect,emitSignal);
}

void VAC::selectFaces(bool emitSignal)
{
    CellSet cellsToSelect;
    foreach(Cell * c, selectedCells())
        if(c->toKeyFace())
            cellsToSelect.insert(c);

    setSelectedCells(cellsToSelect,emitSignal);
}

void VAC::deselectVertices(bool emitSignal)
{
    CellSet cellsToSelect;
    foreach(Cell * c, selectedCells())
        if(!c->toKeyVertex())
            cellsToSelect.insert(c);

    setSelectedCells(cellsToSelect,emitSignal);
}

void VAC::deselectEdges(bool emitSignal)
{
    CellSet cellsToSelect;
    foreach(Cell * c, selectedCells())
        if(!c->toKeyEdge())
            cellsToSelect.insert(c);

    setSelectedCells(cellsToSelect,emitSignal);
}

void VAC::deselectFaces(bool emitSignal)
{
    CellSet cellsToSelect;
    foreach(Cell * c, selectedCells())
        if(!c->toKeyFace())
            cellsToSelect.insert(c);

    setSelectedCells(cellsToSelect,emitSignal);
}

void VAC::prepareDragAndDrop(double x0, double y0, Time time)
{
    draggedVertices_.clear();
    draggedEdges_.clear();

    // do nothing if the highlighted object is not a node object
    if(!hoveredCell_)
        return;

    // get which cells must be dragged
    CellSet cellsToDrag;
    if(hoveredCell_->isSelected() && global()->toolMode() == Global::SELECT)
        cellsToDrag = selectedCells();
    else
        cellsToDrag << hoveredCell_;

    // Partition into three sets of cells
    CellSet cellsNotToKeyframe;
    CellSet cellsToKeyframe;
    foreach(Cell * c, cellsToDrag)
    {
        InbetweenCell * sc = c->toInbetweenCell();
        if(sc)
        {
            if(sc->exists(time))
            {
                cellsToKeyframe << sc;
            }
            else
            {
                cellsNotToKeyframe << sc;
            }
        }
        else
        {
            cellsNotToKeyframe << c;
        }
    }

    // Keyframe cells
    KeyCellSet keyframedCells = keyframe_(cellsToKeyframe,time);

    // Update which cells to drag
    cellsToDrag = cellsNotToKeyframe;
    foreach(KeyCell * c, keyframedCells)
        cellsToDrag << c;
    cellsToDrag = Algorithms::closure(cellsToDrag);

    // todo: add the non-loop edges whose end vertices are dragged
    draggedVertices_ = KeyVertexSet(cellsToDrag);
    draggedEdges_ = KeyEdgeSet(cellsToDrag);

    // prepare drag and drop
    foreach(KeyEdge * iedge, draggedEdges_)
        iedge->geometry()->prepareDragAndDrop();
    foreach(KeyVertex * v, draggedVertices_)
        v->prepareDragAndDrop();

    x0_ = x0;
    y0_ = y0;
}

void VAC::performDragAndDrop(double x, double y)
{
    foreach(KeyEdge * iedge, draggedEdges_)
    {
        iedge->geometry()->performDragAndDrop( x-x0_ , y-y0_ );
        iedge->geometryChanged_();
    }

    foreach(KeyVertex * v, draggedVertices_)
        v->performDragAndDrop( x-x0_ , y-y0_ );


    foreach(KeyVertex * v, draggedVertices_)
        v->correctEdgesGeometry();

    //emit changed();
}

void VAC::completeDragAndDrop()
{
    //emit changed();
    emit checkpoint();
}

void VAC::prepareTemporalDragAndDrop(Time t0)
{
    t0_ = t0;
    draggedKeyCells_ = KeyCellSet(selectedCells());
    draggedKeyCellTime_.clear();

    // TODO: Use smth like Time::min();
    deltaTMin_ = Time(-1000);
    deltaTMax_ = Time(1000);

    foreach(KeyCell * keyCell, draggedKeyCells_)
    {
        Time deltaTMin = keyCell->temporalDragMinTime() - keyCell->time();
        if(deltaTMin_ < deltaTMin)
            deltaTMin_ = deltaTMin;

        Time deltaTMax = keyCell->temporalDragMaxTime() - keyCell->time();
        if(deltaTMax < deltaTMax_)
            deltaTMax_ = deltaTMax;

        draggedKeyCellTime_[keyCell] = keyCell->time();
    }
}

void VAC::performTemporalDragAndDrop(Time t)
{
    Time deltaTime = t-t0_;
    if(deltaTime <= deltaTMin_)
        return;
    if(deltaTime >= deltaTMax_)
        return;

    foreach(KeyCell * keyCell, draggedKeyCells_)
        keyCell->setTime(draggedKeyCellTime_[keyCell] + deltaTime);

    emit changed();
}

void VAC::completeTemporalDragAndDrop()
{
    emit checkpoint();
}



KeyVertex * VAC::split(double x, double y, Time time, bool interactive)
{
    KeyVertex * res = 0;

    if(hoveredCell_)
    {
        KeyVertex * ivertex = hoveredCell_->toKeyVertex();
        KeyEdge * iedge = hoveredCell_->toKeyEdge();
        KeyFace * iface = hoveredCell_->toKeyFace();

        InbetweenVertex * svertex = hoveredCell_->toInbetweenVertex();
        InbetweenEdge * sedge = hoveredCell_->toInbetweenEdge();
        InbetweenFace * sface = hoveredCell_->toInbetweenFace();

        // Create keyframe
        if(svertex)
            ivertex = keyframe_(svertex,time);
        if(sedge)
            iedge = keyframe_(sedge,time);
        if(sface)
            iface = keyframe_(sface,time);

        if(ivertex)
        {
            // if sketch mode, select it
            res = ivertex;
        }
        else if(iedge)
        {
            double radius = 1000;
            iedge->updateSculpt(x, y, radius);
            double s = iedge->geometry()->arclengthOfSculptVertex();
            res = cutEdgeAtVertex_(iedge, s);
        }
        else if(iface)
        {
            // Cut face by adding a steiner cycle, unless we are in sketch
            // mode without planar map mode on
            if(!(global()->toolMode() == Global::SKETCH && !global()->planarMapMode()))
            {
                res = cutFaceAtVertex_(iface, x, y);
            }
        }
    }

    // create something anyway
    if(!res)
    {
        res = newKeyVertex(time);
        res->setPos(Eigen::Vector2d(x,y));
    }

    // create straight line in sketch mode.
    // Note: never happens anymore, as split() is only called in
    if(global()->toolMode() == Global::SKETCH)
    {
        // --------------------------------------------------------------------
        // --------- If non-planar map mode, just create new edges ------------
        // --------------------------------------------------------------------

        if(!global()->planarMapMode())
        {
            // If a vertex is selected, create a new edge between this
            // selected vertex and res, where res is either the vertex that
            // was highlighted, or a newly created vertex
            // This possibly craete many straight lines at once
            KeyVertexSet selectedVertices = selectedCells();
            KeyEdgeSet newEdges;
            foreach(KeyVertex * selectedVertex, selectedVertices)
            {
                newEdges << newKeyEdge(time, selectedVertex, res, 0, global()->edgeWidth());
            }
        }

        // --------------------------------------------------------------------
        // ---- If planar map mode, cut edges/faces with these new edges --------
        // --------------------------------------------------------------------

        if(global()->planarMapMode())
        {
            KeyVertexSet selectedVertices = selectedCells();

            foreach(KeyVertex * selectedVertex, selectedVertices)
            {
                // Tolerance accounting for floating point errors
                double tolerance = 1e-6;

                // Emulate begin/continue/end PMR sketch to get same behaviour

                // Begin
                timeInteractivity_ = time;
                sketchedEdge_ = new LinearSpline(ds_);
                sketchedEdge_->beginSketch(EdgeSample(selectedVertex->pos()[0],selectedVertex->pos()[1],global()->edgeWidth()));
                hoveredFaceOnMouseRelease_ = 0;
                hoveredFaceOnMousePress_ = 0;

                // Continue
                sketchedEdge_->continueSketch(EdgeSample(res->pos()[0], res->pos()[1], global()->edgeWidth()));

                // End
                sketchedEdge_->endSketch();
                sketchedEdge_->resample();
                CellSet allCells = cells();
                //facesToConsiderForCutting_ = KeyFaceSet(allCells);
                insertSketchedEdgeInVAC(tolerance, false);
                delete sketchedEdge_;
                sketchedEdge_ = 0;
            }
        }

        // In any case, select the newly created vertex
        CellSet newSeletedCells;
        newSeletedCells << res;
        setSelectedCells(newSeletedCells);
    }
    else
    {
        // nothing
    }

    if(interactive)
    {
        emit needUpdatePicking();
        emit changed();
        emit checkpoint();
    }

    return res;
}

bool VAC::check() const
{
    foreach(Cell * c, cells_)
        if(!(c->check()))
            return false;
    return true;
}

bool VAC::checkContains(const Cell * c) const
{
    int id = c->id();
    return (cells_.contains(id)) && (cells_[id] == c);
}

void VAC::updateToBePaintedFace(double x, double y, Time time)
{
    // Init face
    if(!toBePaintedFace_)
        toBePaintedFace_ = new PreviewKeyFace;

    // Erase previous results
    toBePaintedFace_->clear();

    // Check if highlighted cell
    if(hoveredCell())
    {
        // In this case, painting would change the color of the
        // highlighted cell, instead of creating a new face
        return;
    }

    // From here, we try to find a list of cycles such that
    // the corresponding face would intersect with the cursor

    // Compute distances to all edges
    QMap<KeyEdge*,EdgeGeometry::ClosestVertexInfo> distancesToEdges;
    foreach(KeyEdge * e, instantEdges())
        distancesToEdges[e] = e->geometry()->closestPoint(x,y);

    // First, we try to create such a face assuming that the
    // VGC is actually planar (cells are not overlapping).
    bool foundPlanarFace = false;
    {
        // Find external boundary: the closest planar cycle containing mouse cursor
        QSet<KeyEdge*> potentialExternalBoundaryEdges;
        foreach(KeyEdge* e, instantEdges())
        {
            if(e->exists(time))
                potentialExternalBoundaryEdges.insert(e);
        }
        PreviewKeyFace externalBoundary;
        bool foundExternalBoundary = false;
        while(!(foundExternalBoundary || potentialExternalBoundaryEdges.isEmpty()))
        {
            // Find closest potential edge
            KeyEdge * closestPotentialExternalBoundaryEdge = 0;
            EdgeGeometry::ClosestVertexInfo cvi;
            cvi.s = 0;
            cvi.d = std::numeric_limits<double>::max();
            foreach(KeyEdge * e, potentialExternalBoundaryEdges)
            {
                EdgeGeometry::ClosestVertexInfo cvi_e = distancesToEdges[e];
                if(cvi_e.d < cvi.d) // TODO: cvi_e.d could be NaN, in which case it returns false, and closestPotentialExternalBoundaryEdge can be null, resulting in a segfault 8 lines below.
                {
                    closestPotentialExternalBoundaryEdge = e;
                    cvi = cvi_e;
                }
            }

            // Find direction of halfedge
            Eigen::Vector2d der = closestPotentialExternalBoundaryEdge->geometry()->der(cvi.s);
            double cross = der[0] * (y - cvi.p.y()) - der[1] * (x - cvi.p.x());
            KeyHalfedge h(closestPotentialExternalBoundaryEdge, (cross>0 ? false : true) ); // be careful here, canvas is left-handed

            // Find potential external boundary
            if(closestPotentialExternalBoundaryEdge->isClosed())
            {
                KeyEdgeSet edgeSet;
                edgeSet << closestPotentialExternalBoundaryEdge;
                Cycle cycle(edgeSet);
                if(cycle.isValid())
                {
                    externalBoundary << cycle;
                    if(externalBoundary.intersects(x,y))
                    {
                        foundExternalBoundary = true;
                    }
                    else
                    {
                        potentialExternalBoundaryEdges.remove(closestPotentialExternalBoundaryEdge);
                        externalBoundary.clear();
                    }
                }
                else
                {
                    potentialExternalBoundaryEdges.remove(closestPotentialExternalBoundaryEdge);
                }
            }
            else // i.e., if closestPotentialExternalBoundaryEdge is open
            {
                // First halfedge of non-simple-cycle
                KeyHalfedge h0 = h;
                QList<KeyHalfedge> potentialPlanarCycle;
                potentialPlanarCycle << h;

                // Find the corresponding planar map cycle
                int maxIter = 2 * potentialExternalBoundaryEdges.size() + 2;
                bool foundPotentialPlanarCycle = false;
                for(int i=0; i<maxIter; i++)
                {
                    // Find next halfedge in cycle
                    h = h.next();

                    // Check it has not already been rejected
                    if(!potentialExternalBoundaryEdges.contains(h.edge))
                    {
                        break;
                    }

                    // Test if cycle completed or not
                    if(h == h0)
                    {
                        // Cycle completed: leave loop
                        foundPotentialPlanarCycle = true;
                        break;
                    }
                    else
                    {
                        // Cycle not completed: insert and iterate
                        potentialPlanarCycle << h;
                    }
                }

                // If not found (maxIter reached or edge already rejected)
                if(!foundPotentialPlanarCycle)
                {
                    foreach(KeyHalfedge he, potentialPlanarCycle)
                    {
                        potentialExternalBoundaryEdges.remove(he.edge);
                    }
                }
                else
                {
                    Cycle cycle(potentialPlanarCycle);
                    if(cycle.isValid())
                    {
                        externalBoundary << cycle;
                        if(externalBoundary.intersects(x,y))
                        {
                            foundExternalBoundary = true;
                        }
                        else
                        {
                            foreach(KeyHalfedge he, potentialPlanarCycle)
                            {
                                potentialExternalBoundaryEdges.remove(he.edge);
                            }
                            externalBoundary.clear();
                        }
                    }
                    else
                    {
                        foreach(KeyHalfedge he, potentialPlanarCycle)
                        {
                            potentialExternalBoundaryEdges.remove(he.edge);
                        }
                    }
                }
            } // end if/then/else "closestPotentialExternalBoundaryEdge is closed?"

        }// end while( !( found external boundary || isEmpty) )

        // We left the while loop, so either we found an external boundary, or there's no hope to find one
        if(foundExternalBoundary)
        {
            // Great, so we know we have a valid planar face!
            // Plus, it's already stored as a PreviewFace :)
            *toBePaintedFace_ = externalBoundary;
            foundPlanarFace = true;

            // Now, let's try to add holes to the external boundary
            QSet<KeyEdge*> potentialHoleEdges;
            foreach(KeyEdge* e, instantEdges())
            {
                if(e->exists(time))
                    potentialHoleEdges.insert(e);
            }
            CellSet cellsInExternalBoundary = externalBoundary.cycles()[0].cells();
            KeyEdgeSet edgesInExternalBoundary = cellsInExternalBoundary;
            foreach(KeyEdge * e, edgesInExternalBoundary)
                potentialHoleEdges.remove(e);
            QList<PreviewKeyFace> holes;
            while(!potentialHoleEdges.isEmpty())
            {
                // Ordered by distance to mouse cursor p, add planar cycles gamma which:
                //   - Do not contain p
                //   - Are contained in external boundary
                //   - Are not contained in holes already added
                addHoleToPaintedFace(potentialHoleEdges, *toBePaintedFace_, distancesToEdges, x, y);
            }
        }
        else
        {
            // If we haven't found an external boundary, then we can't find a planar face
            // Just continue the method to find a non-planar face
            foundPlanarFace = false;
        }
    } // end of trying to find a planar face

    if(foundPlanarFace)
    {
        // Great, nothing to do! Everything has already been taken care of.
    }
    else
    {
        // TODO: try to find any valid face, even if it's not planar
    }
}

Cell * VAC::paint(double /*x*/, double /*y*/, Time /*time*/)
{
    // The created face, if any
    Cell * res = 0;

    // Paint existing cell
    if(hoveredCell())
    {
        hoveredCell()->setColor(global()->faceColor());
        res = hoveredCell();
    }

    // Create a new face
    else if(toBePaintedFace_->numCycles() > 0)
    {
        res = newKeyFace(toBePaintedFace_->cycles());
    }

    if (res)
    {
        emit needUpdatePicking();
        emit changed();
        emit checkpoint();
    }

    // Return
    return res;
}

void VAC::test()
{
    // This function is for debug purposes
    // Insert below the code you want to execute by pressing "T" within VPaint
    // Note: for this to work, you also have to uncomment in MainWindow.cpp the line:
    //             menuEdit->addAction(actionTest);

    emit needUpdatePicking();
    emit changed();
    emit checkpoint();
}

}

// GUI

#include <QAction>
#include <QToolBar>
#include "../Scene.h"
#include "../ColorSelector.h"

namespace VectorAnimationComplex
{

void VAC::populateToolBar(QToolBar * /*toolBar*/, Scene * /*scene*/)
{
}

}



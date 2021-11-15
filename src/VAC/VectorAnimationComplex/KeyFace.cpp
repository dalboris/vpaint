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

#include "../XmlStreamReader.h"
#include "../XmlStreamWriter.h"
#include <QTextStream>
#include <QtDebug>
#include <QMessageBox>
#include <array>
#include <vector>

#include "../SaveAndLoad.h"
#include "../Random.h"

#include "EdgeGeometry.h"

#include "KeyVertex.h"
#include "KeyEdge.h"
#include "KeyFace.h"
#include "../DevSettings.h"
#include "../Global.h"

#include "../OpenGL.h"

#if defined(__APPLE__) && defined(__MACH__)
#  include <OpenGL/glu.h>
#else
#  include <GL/glu.h>
#endif

// ------- Unnamed namespace for non-friend non-member helper functions -------

namespace
{

using namespace VectorAnimationComplex;

// Active tesselator
GLUtesselator * tobj = 0;

// Offline tesselator: outputs the triangulation as offlineTessTriangles
GLUtesselator * tobjOffline = 0;
Triangles offlineTessTriangles;
GLenum offlineTessWhich;
int offlineTessIter;
double offlineTessAX, offlineTessAY;
double offlineTessBX, offlineTessBY;
double offlineTessCX, offlineTessCY;

#ifdef _WIN32
#define CALLBACK __stdcall
#else
#define CALLBACK
#endif

void CALLBACK offlineTessBegin(GLenum which)
{
    offlineTessWhich = which;
    offlineTessIter = 0;
}

void CALLBACK offlineTessEnd(void)
{
}

void CALLBACK offlineTessError(GLenum errorCode)
{
   const GLubyte *estring;
   estring = gluErrorString(errorCode);

   qDebug() << "Tessellation Error:" << estring;
}

void CALLBACK offlineTessVertex(GLvoid *vertex)
{
    const GLdouble *pointer = (GLdouble *) vertex;

    if(offlineTessWhich == GL_TRIANGLES)
    {
        if(offlineTessIter == 0)
        {
            offlineTessAX = pointer[0];
            offlineTessAY = pointer[1];
            offlineTessIter = 1;
        }
        else if(offlineTessIter == 1)
        {
            offlineTessBX = pointer[0];
            offlineTessBY = pointer[1];
            offlineTessIter = 2;
        }
        else
        {
            offlineTessCX = pointer[0];
            offlineTessCY = pointer[1];
            offlineTessIter = 0;

            offlineTessTriangles.append(offlineTessAX, offlineTessAY,
                                        offlineTessBX, offlineTessBY,
                                        offlineTessCX, offlineTessCY);
        }
    }
    else if(offlineTessWhich == GL_TRIANGLE_FAN)
    {
        if(offlineTessIter == 0)
        {
            offlineTessAX = pointer[0];
            offlineTessAY = pointer[1];
            offlineTessIter = 1;
        }
        else if(offlineTessIter == 1)
        {
            offlineTessBX = pointer[0];
            offlineTessBY = pointer[1];
            offlineTessIter = 2;
        }
        else
        {
            offlineTessCX = pointer[0];
            offlineTessCY = pointer[1];

            offlineTessTriangles.append(offlineTessAX, offlineTessAY,
                                        offlineTessBX, offlineTessBY,
                                        offlineTessCX, offlineTessCY);

            offlineTessBX = offlineTessCX;
            offlineTessBY = offlineTessCY;
        }
    }
    else if(offlineTessWhich == GL_TRIANGLE_STRIP)
    {
        if(offlineTessIter == 0)
        {
            offlineTessAX = pointer[0];
            offlineTessAY = pointer[1];
            offlineTessIter = 1;
        }
        else if(offlineTessIter == 1)
        {
            offlineTessBX = pointer[0];
            offlineTessBY = pointer[1];
            offlineTessIter = 2;
        }
        else if(offlineTessIter == 2)
        {
            offlineTessCX = pointer[0];
            offlineTessCY = pointer[1];

            offlineTessTriangles.append(offlineTessAX, offlineTessAY,
                                        offlineTessBX, offlineTessBY,
                                        offlineTessCX, offlineTessCY);

            offlineTessAX = offlineTessCX;
            offlineTessAY = offlineTessCY;
            offlineTessIter = 3;
        }
        else
        {
            offlineTessCX = pointer[0];
            offlineTessCY = pointer[1];

            offlineTessTriangles.append(offlineTessAX, offlineTessAY,
                                        offlineTessBX, offlineTessBY,
                                        offlineTessCX, offlineTessCY);

            offlineTessBX = offlineTessCX;
            offlineTessBY = offlineTessCY;
            offlineTessIter = 2;
        }
    }
    else if(offlineTessWhich == GL_LINE_LOOP)
    {
    }
}

void CALLBACK offlineTessCombine(GLdouble coords[3],
                        GLdouble * /*vertex_data*/ [4],
                        GLfloat /*weight*/ [4], GLdouble **dataOut )
{
   GLdouble * vertex = (GLdouble *) malloc(6 * sizeof(GLdouble));
   vertex[0] = coords[0];
   vertex[1] = coords[1];
   vertex[2] = coords[2];

   *dataOut = vertex;
}

typedef std::vector< std::vector< std::array<GLdouble, 3> > > PolygonData;

PolygonData createPolygonData(const QList<Cycle> & cycles)
{
    PolygonData vertices;
    for(int k=0; k<cycles.size(); ++k)      // for each cycle
    {
        vertices << std::vector< std::array<GLdouble, 3> >(); // create a contour data

        for(int i=0; i<cycles[k].size(); ++i) // for each edge in the cycle
        {
            QList<Eigen::Vector2d> & sampling = cycles[k][i].edge->geometry()->sampling();
            if(cycles[k][i].side)
            {
                int last = sampling.size()-1;
                if(i+1<cycles[k].size())
                {
                    // if there is a following edge, then don't insert last vertex
                    // otherwise it will be duplicated
                    last--;
                }
                for(int j=0; j<=last; ++j)
                {
                    std::array<GLdouble, 3> a = {sampling[j][0], sampling[j][1], 0};
                    vertices.back().emplace_back(a);
                }
            }
            else
            {
                int first = 0;
                if(i+1<cycles[k].size())
                {
                    // if there is a following edge, then don't insert last vertex
                    // otherwise it will be duplicated
                    first++;
                }
                for(int j=sampling.size()-1; j>=first; --j)
                {
                    std::array<GLdouble, 3> a = {sampling[j][0], sampling[j][1], 0};
                    vertices.back().emplace_back(a);
                }
            }
        }
    }

    return vertices;
}

void computeTrianglesFromCycles(const QList<Cycle> & cycles, Triangles & triangles)
{

    // Creating polygon data for GLU tesselator
    PolygonData vertices = createPolygonData(cycles);

    // Creating the GLU tesselation object
    if(!tobjOffline)
    {
        tobjOffline = gluNewTess();
    }
    if(tobj != tobjOffline)
    {
        tobj = tobjOffline;

        gluTessCallback(tobj, GLU_TESS_VERTEX,
                        (GLvoid (CALLBACK*) ()) &offlineTessVertex);
        gluTessCallback(tobj, GLU_TESS_BEGIN,
                        (GLvoid (CALLBACK*) ()) &offlineTessBegin);
        gluTessCallback(tobj, GLU_TESS_END,
                        (GLvoid (CALLBACK*) ()) &offlineTessEnd);
        gluTessCallback(tobj, GLU_TESS_ERROR,
                        (GLvoid (CALLBACK*) ()) &offlineTessError);
        gluTessCallback(tobj, GLU_TESS_COMBINE,
                        (GLvoid (CALLBACK*) ()) &offlineTessCombine);
    }

    // Using the tesselation object
    gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);

    // Specifying data
    offlineTessTriangles.clear();
    gluTessBeginPolygon(tobj, NULL);
    {
        for(auto & vec: vertices) // for each cycle
        {
            gluTessBeginContour(tobj); // draw a contour
            {
                for(auto & v: vec) // for each vertex in cycle
                {
                    // safeguard against NaN and other oddities
                    const double MAX_VALUE = 10000;
                    const double MIN_VALUE = -10000;
                    if( v[0] > MIN_VALUE &&
                        v[0] < MAX_VALUE &&
                        v[1] > MIN_VALUE &&
                        v[1] < MAX_VALUE &&
                        v[2] > MIN_VALUE &&
                        v[2] < MAX_VALUE )
                    {
                        gluTessVertex(tobj, v.data(), v.data()); // send vertex
                    }
                    else
                    {
                        qDebug() << "ignored vertex" << v[0]  << v[1]  << v[2] << "for tesselation";
                    }
                }
            }
            gluTessEndContour(tobj);
        }
    }
    gluTessEndPolygon(tobj);

    // Tranfer to member data
    triangles = offlineTessTriangles;
}

}


namespace VectorAnimationComplex
{

// ------------------------------- Key Face -------------------------------

KeyFace::KeyFace(VAC * vac, const Time & t) :
    Cell(vac),
    KeyCell(vac, t),
    FaceCell(vac)
{
    initColor_();
}

KeyFace::KeyFace(VAC * vac, const Cycle & cycle) :
    Cell(vac),
    KeyCell(vac, cycle.time()),
    FaceCell(vac)
{
    addCycle(cycle);
    initColor_();
}

KeyFace::KeyFace(VAC * vac, const QList<Cycle> & cycles) :
    Cell(vac),
    KeyCell(vac, cycles[0].time()),
    FaceCell(vac)
{
    addCycles(cycles);
    initColor_();
}

void KeyFace::initColor_()
{
    // Random color
    /*
        // using hsv to rgb
        double H = Random::random(60./60.,200./60.);
        double S = 0.2;
        double V = 1;

        double C = V*S;
        double X = C * ( 1 - std::abs(std::fmod(H,2)-1) );
        double R1, G1, B1;
        if(H<1) { R1=C; G1=X; B1=0;}
        else if(H<2) { R1=X; G1=C; B1=0;}
        else if(H<3) { R1=0; G1=C; B1=X;}
        else if(H<4) { R1=0; G1=X; B1=C;}
        else if(H<5) { R1=X; G1=0; B1=C;}
        else { R1=C; G1=0; B1=X;}
        double m = V - C;
        color_[0] = R1+m;
        color_[1] = G1+m;
        color_[2] = B1+m;
        color_[3] = 1;
    */

    // Color from GUI
    setColor(global()->faceColor());
}

void KeyFace::clearCycles_()
{
    const auto& spatialBoundaryCells = spatialBoundary();
    for(Cell * cell: spatialBoundaryCells)
        removeMeFromSpatialStarOf_(cell);

    cycles_.clear();
    processGeometryChanged_();
}

void KeyFace::setCycles(const QList<Cycle> & cycles)
{
    clearCycles_();
    addCycles(cycles);
}

void KeyFace::addCycles(const QList<Cycle> & cycles)
{
    for(int i=0; i<cycles.size(); ++i)
        addCycle(cycles[i]);
}

void KeyFace::addCycle(const Cycle & cycle)
{
    cycles_ << cycle;
    const auto& keyCycleCells = cycle.cells();
    for(KeyCell * cell: keyCycleCells)
        addMeToSpatialStarOf_(cell);
    processGeometryChanged_();
}

void KeyFace::triangulate_(Time time, Triangles & out) const
{
    out.clear();
    if (exists(time))
        computeTrianglesFromCycles(cycles_, out);
}

QList<QList<Eigen::Vector2d> > KeyFace::getSampling(Time /*time*/) const
{
    QList<QList<Eigen::Vector2d> > res;
    PolygonData data = createPolygonData(cycles_);

    for(unsigned int k=0; k<data.size(); ++k) // for each cycle
    {
        res << QList<Eigen::Vector2d>();
        for(unsigned int i=0; i<data[k].size(); ++i) // for each edge in the cycle
        {
            res[k] << Eigen::Vector2d(data[k][i][0], data[k][i][1]);
        }
    }

    return res;
}

QString KeyFace::xmlType_() const
{
    return "face";
}

void KeyFace::write_(XmlStreamWriter & xml) const
{
    // Base classes
    Cell::write_(xml);
    KeyCell::write_(xml);
    FaceCell::write_(xml);

    // Cycles
    QString cyclesstring;
    for(int i=0; i<cycles_.size(); ++i)
    {
        if(i>0)
            cyclesstring += " ";
        cyclesstring += cycles_[i].toString();
    }
    xml.writeAttribute("cycles", cyclesstring);
}

KeyFace::KeyFace(VAC * vac, XmlStreamReader & xml) :
    Cell(vac, xml),
    KeyCell(vac, xml),
    FaceCell(vac, xml)
{
    // Cycles
    QString str;
    QStringRef d = xml.attributes().value("cycles");
    bool opened = false;
    for(int i=0; i<d.length(); ++i)
    {
        QChar c = d.at(i);
        if(c == '[')
            opened = true;
        if(opened)
            str += c;
        if(c==']')
        {
            cycles_ << Cycle();
            cycles_.last().fromString(str);
            opened = false;
            str.clear();
        }
    }
}

void KeyFace::save_(QTextStream & out)
{
    // Base classes
    Cell::save_(out);
    KeyCell::save_(out);
    FaceCell::save_(out);

    // Cycles
    out << Save::newField("Cycles") << cycles_;
}

KeyFace::KeyFace(VAC * vac, QTextStream & in) :
    Cell(vac, in),
    KeyCell(vac, in),
    FaceCell(vac, in)
{
    Field field;

    // Cycles
    in >> /*field >>*/ cycles_; // Reason of comment: see Cell::Cell(VAC * vac, QTextStream & in)
}

void KeyFace::read2ndPass()
{
    // Base classes
    Cell::read2ndPass();
    KeyCell::read2ndPass();
    FaceCell::read2ndPass();

    // Cycles
    for(int i=0; i<cycles_.size(); ++i)
        cycles_[i].convertTempIdsToPointers(vac());
}

KeyFace::~KeyFace()
{
}

CellSet KeyFace::spatialBoundary() const
{
    CellSet res;

    for(int i=0; i<cycles_.size(); ++i)
    {
        CellSet cells = cycles_[i].cells();
        res.unite(cells);
    }

    return res;
}

void KeyFace::updateBoundary_impl(KeyEdge * oldEdge, const KeyEdgeList & newEdges)
{
    for(int i=0; i<cycles_.size(); ++i)
        cycles_[i].replaceEdges(oldEdge, newEdges);
}

// Update boundary
void KeyFace::updateBoundary_impl(KeyVertex * oldVertex, KeyVertex * newVertex)
{
    for(int i=0; i<cycles_.size(); ++i)
        cycles_[i].replaceVertex(oldVertex, newVertex);
}

void KeyFace::updateBoundary_impl(const KeyHalfedge & oldHalfedge, const KeyHalfedge & newHalfedge)
{
    for(int i=0; i<cycles_.size(); ++i)
        cycles_[i].replaceHalfedge(oldHalfedge, newHalfedge);
}

KeyFace * KeyFace::clone()
{
    return new KeyFace(this);
}

void KeyFace::remapPointers(VAC * newVAC)
{
    Cell::remapPointers(newVAC);
    KeyCell::remapPointers(newVAC);
    FaceCell::remapPointers(newVAC);

    for(int i=0; i<cycles_.size(); ++i)
        cycles_[i].remapPointers(newVAC);
}

KeyFace::KeyFace(KeyFace * other) :
    Cell(other),
    KeyCell(other),
    FaceCell(other)
{
    cycles_ = other->cycles_;
}

bool KeyFace::check_() const
{
    // todo
    return true;
}

// --------------------------- Preview Key Face ---------------------------


PreviewKeyFace::PreviewKeyFace() :
    cycles_()
{
}

PreviewKeyFace::PreviewKeyFace(const Cycle & cycle) :
    cycles_()
{
    cycles_ << cycle;
    computeTriangles_();
}

PreviewKeyFace::PreviewKeyFace(const QList<Cycle> & cycles) :
    cycles_(cycles)
{
}

void PreviewKeyFace::clear()
{
    cycles_.clear();
    triangles_.clear();
}

PreviewKeyFace & PreviewKeyFace::operator<< (const Cycle & cycle)
{
    cycles_ << cycle;
    computeTriangles_();
    return *this;
}

void PreviewKeyFace::computeTriangles_()
{
    computeTrianglesFromCycles(cycles_,triangles_);
}

bool PreviewKeyFace::intersects(double x, double y) const
{
    return triangles_.intersects(Eigen::Vector2d(x,y));
}

void PreviewKeyFace::draw(ViewSettings & /*viewSettings*/)
{
    // Setting the color
    glColor4d(0.9,0.9,0.9,0.8);

    // Drawing
    triangles_.draw();
}

}

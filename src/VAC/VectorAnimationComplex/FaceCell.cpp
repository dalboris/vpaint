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

#include "FaceCell.h"

#include <QTextStream>
#include <QtDebug>

#include "../SaveAndLoad.h"
#include "../DevSettings.h"
#include "../Global.h"
#include "../OpenGL.h"

#if defined(__APPLE__) && defined(__MACH__)
#  include <OpenGL/glu.h>
#else
#  include <GL/glu.h>
#endif

namespace VectorAnimationComplex
{

FaceCell::FaceCell(VAC * vac) :
    Cell(vac)
{
    // highlighted/selected color
    colorSelected_[0] = 1;
    colorSelected_[1] = 0.5;
    colorSelected_[2] = 0.5;
    colorSelected_[3] = 1;
}

FaceCell::FaceCell(VAC * vac, XmlStreamReader & xml) :
    Cell(vac, xml)
{
    // highlighted/selected color
    colorSelected_[0] = 1;
    colorSelected_[1] = 0.5;
    colorSelected_[2] = 0.5;
    colorSelected_[3] = 1;
}

FaceCell::FaceCell(VAC * vac, QTextStream & in) :
    Cell(vac, in)
{
    // highlighted/selected color
    colorSelected_[0] = 1;
    colorSelected_[1] = 0.5;
    colorSelected_[2] = 0.5;
    colorSelected_[3] = 1;
}

void FaceCell::drawRawTopology(Time time, ViewSettings & viewSettings)
{
    if(viewSettings.drawTopologyFaces())
        triangles(time).draw();
}

bool FaceCell::isPickableCustom(Time /*time*/) const
{
    const bool areFacesPickable = true;
    if(areFacesPickable && global()->toolMode() == Global::SELECT)
        return true;
    else if(global()->toolMode() == Global::PAINT)
        return true;
    else if(global()->toolMode() == Global::SKETCH) // to detect which faces are hovered in planar map mode
        return true;
    else
        return false;
}

void FaceCell::computeOutlineBoundingBox_(Time t, BoundingBox & out) const
{
    out = boundingBox(t);
}

void FaceCell::exportSVG(QTextStream & out, const VectorExportSettings & /*settings*/, Time t)
{
    // Get polygon data
    QList< QList<Eigen::Vector2d> > samples = getSampling(t);

    // Write file
    out << "<path d=\"";
    for(int k=0; k<samples.size(); ++k) // for each cycle
    {
        if(samples[k].size() < 2)
            continue;

        Eigen::Vector2d v0 = samples[k][0];
        out << "M " << v0[0] << "," << v0[1] << " ";

        for(int i=0; i<samples[k].size(); ++i) // for each vertex in cycle
        {
            Eigen::Vector2d v = samples[k][i];

            // safeguard against NaN and other oddities
            const double MAX_VALUE = 10000;
            const double MIN_VALUE = -10000;
            if( v[0] > MIN_VALUE &&
                    v[0] < MAX_VALUE &&
                    v[1] > MIN_VALUE &&
                    v[1] < MAX_VALUE )
            {
                out << "L " << v[0] << "," << v[1] << " ";
            }
        }
        out << "Z ";
    }
    out << "\" style=\""
        << "fill:rgb("
        << (int) (color_[0]*255) << ","
        << (int) (color_[1]*255) << ","
        << (int) (color_[2]*255) << ");"
        << "fill-opacity:" << color_[3] << ";"
        << "fill-rule:evenodd;stroke:none\" />\n";

}

void FaceCell::read2ndPass()
{
}

void FaceCell::save_(QTextStream & /*out*/)
{
}

FaceCell::~FaceCell()
{
}

FaceCell::FaceCell(FaceCell * other) :
    Cell(other)
{
}

void FaceCell::remapPointers(VAC * /*newVAC*/)
{
}

bool FaceCell::checkFace_() const
{
    // todo
    return true;
}

namespace
{

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

// Typedef for pointer-to-function taking zero arguments and returning a
// `void`. This is the type of the argument expected by gluTessCallback().
//
typedef void (CALLBACK *TessCallback)();

// Casts from any pointer-to-function to TessCallback.
//
// This avoids warnings such as:
//
//     cast between incompatible function types from `void (*)(GLenum)` to `GLvoid (*)()`
//
// The problem is that the GLU API is not type-correct: gluTessCallback()
// always takes as argument a `GLvoid (*)()` (callback with no arguments),
// while in fact we do need to pass a callback that takes arguments, so the
// compiler is right to complain.
//
// As a workaround, we do an intermediate cast to `void*`.
//
template<typename Callback>
TessCallback castToTessCallback(Callback c) {
    void* voidPtr = (void*)(c);
    return (TessCallback)(voidPtr);
}

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

} // namespace

namespace detail {

void tesselatePolygon(const PolygonData & vertices, Triangles & triangles) {

    // Creating the GLU tesselation object
    if(!tobjOffline)
    {
        tobjOffline = gluNewTess();
    }
    if(tobj != tobjOffline)
    {
        tobj = tobjOffline;

        gluTessCallback(tobj, GLU_TESS_VERTEX, castToTessCallback(&offlineTessVertex));
        gluTessCallback(tobj, GLU_TESS_BEGIN, castToTessCallback(&offlineTessBegin));
        gluTessCallback(tobj, GLU_TESS_END, castToTessCallback(&offlineTessEnd));
        gluTessCallback(tobj, GLU_TESS_ERROR, castToTessCallback(&offlineTessError));
        gluTessCallback(tobj, GLU_TESS_COMBINE, castToTessCallback(&offlineTessCombine));
    }

    // Using the tesselation object
    gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);

    // Specifying data
    offlineTessTriangles.clear();
    gluTessBeginPolygon(tobj, NULL);
    {
        for(const auto & vec: vertices) // for each cycle
        {
            gluTessBeginContour(tobj); // draw a contour
            {
                for(const auto & v: vec) // for each vertex in cycle
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
                        GLdouble* location = (GLdouble*)v.data();
                        GLvoid* data = (GLvoid*)v.data();
                        gluTessVertex(tobj, location, data); // send vertex
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

} // namespace detail

}

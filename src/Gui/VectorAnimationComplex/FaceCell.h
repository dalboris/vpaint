// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef VAC_FACE_CELL_H
#define VAC_FACE_CELL_H

#include "Cell.h"
#include "Triangles.h"

namespace VectorAnimationComplex
{

class FaceCell: virtual public Cell
{
public:
    FaceCell(VAC * vac);

    // Drawing
    virtual void triangulate(Time time, Triangles & out);
    Triangles & triangles(Time time);
    void drawRaw(Time time, ViewSettings & viewSettings);
    void drawRawTopology(Time time, ViewSettings & viewSettings);

    // Get sampling of the boundary
    virtual QList< QList<Eigen::Vector2d> > getSampling(Time time) const = 0;

    // Intersection test
    virtual bool intersectsRectangle(Time t, double x0, double x1, double y0, double y1);

    // Export SVG
    virtual void exportSVG(Time t, QTextStream & out);

protected:
    virtual ~FaceCell()=0;

private:
    // Cached triangulations (the integer represent a 1/60th of frame)
    QMap<int,Triangles> triangles_;
    void clearCachedGeometry_();

    // Trusting operators
    friend class Operator;
    bool checkFace_() const;

    virtual bool isPickableCustom(Time time) const;

// --------- Cloning, Assigning, Copying, Serializing ----------

protected:
    virtual void remapPointers(VAC * newVAC);
    virtual void save_(QTextStream & out);
    FaceCell(FaceCell * other);
    virtual QString stringType() const {return "FaceCell";}
    FaceCell(VAC * vac, QTextStream & in);
    virtual void read2ndPass();
    FaceCell(VAC * vac, XmlStreamReader & xml);

};

}


#endif

// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

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

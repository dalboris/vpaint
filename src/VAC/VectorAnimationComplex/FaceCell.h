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

    // Export SVG
    virtual void exportSVG(Time t, QTextStream & out);

protected:
    virtual ~FaceCell()=0;

private:
    // Trusting operators
    friend class Operator;
    bool checkFace_() const;

    virtual bool isPickableCustom(Time time) const;

    // Implementation of outline bounding box for both KeyFace and InbetweenFace
    void computeOutlineBoundingBox_(Time t, BoundingBox & out) const;

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

namespace detail {

typedef std::vector< std::vector< std::array<double, 3> > > PolygonData;
void tesselatePolygon(const PolygonData & polygon, Triangles & triangles);

} // namespace detail

}

#endif

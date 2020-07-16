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

#ifndef VAC_INSTANT_CELL_H
#define VAC_INSTANT_CELL_H

#include "Cell.h"

namespace VectorAnimationComplex
{

class KeyCell: virtual public Cell
{
public:
    // -------------- Public standard interface -------------

    // Constructor and destructor
    KeyCell(VAC * vac, Time time);

    // drawing
    virtual void drawRaw3D(View3DSettings & viewSettings);

    // Time info
    int frame() const { return time_.frame(); }
    double floatTime() const { return time_.floatTime(); }
    Time time() const { return time_; }

    Time temporalDragMinTime() const;
    Time temporalDragMaxTime() const;
    void setTime(Time time);

    // Reimplement
    bool exists(Time time) const;
    bool isBefore(Time time) const;
    bool isAfter(Time time) const;
    bool isAt(Time time) const;

    // Topological Navigation Information
    KeyCellSet beforeCells() const;
    KeyCellSet afterCells() const;

    // Bounding box
    using Cell::boundingBox;
    using Cell::outlineBoundingBox;
    BoundingBox boundingBox() const;
    BoundingBox outlineBoundingBox() const;

private:
    Time time_;

    // Trusting operators
    friend class VAC;
    friend class Operator;
    bool checkKey_() const;

protected:
    virtual ~KeyCell()=0;

// --------- Cloning, Assigning, Copying, Serializing ----------


protected:
    KeyCell(VAC * vac, QTextStream & in);
    virtual QString stringType() const {return "KeyCell";}
    virtual void read2ndPass();
    virtual void remapPointers(VAC * newVAC);
    virtual void save_(QTextStream & out);
    KeyCell(KeyCell * other);
    KeyCell(VAC * vac, XmlStreamReader & xml);
    virtual void write_(XmlStreamWriter & xml) const;
};

}

#endif

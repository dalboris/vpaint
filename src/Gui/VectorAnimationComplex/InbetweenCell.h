// Copyright (C) 2012-2019 The VPaint Developers
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

#ifndef VAC_ANIMATED_CELL_H
#define VAC_ANIMATED_CELL_H

#include "Cell.h"

namespace VectorAnimationComplex
{

class InbetweenCell: virtual public Cell
{
public:
    InbetweenCell(VAC * vac);

    // Time info
    Time beforeTime() const;
    Time afterTime() const;

    bool exists(Time time) const;
    bool isBefore(Time time) const;
    bool isAfter(Time time) const;
    bool isAt(Time time) const;

    // Temporal boundary
    virtual KeyCellSet beforeCells() const=0;
    virtual KeyCellSet afterCells() const=0;

    // Bounding box
    using Cell::boundingBox;
    using Cell::outlineBoundingBox;
    BoundingBox boundingBox() const;
    BoundingBox outlineBoundingBox() const;

private:
    // Trusting operators
    friend class Operator;
    bool checkAnimated_() const;


// --------- Cloning, Assigning, Copying, Serializing ----------


protected:
    virtual ~InbetweenCell()=0;
    InbetweenCell(VAC * vac, QTextStream & in);
    virtual QString stringType() const {return "InbetweenCell";}
    virtual void read2ndPass();
    virtual void remapPointers(VAC * newVAC);
    virtual void save_(QTextStream & out);
    InbetweenCell(InbetweenCell * other);
    InbetweenCell(VAC * vac, XmlStreamReader & xml);
    virtual void write_(XmlStreamWriter & xml) const;
};
        
}

#endif

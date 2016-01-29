// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

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
    BoundingBox boundingBox() const;

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

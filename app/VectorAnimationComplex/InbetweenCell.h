// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

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

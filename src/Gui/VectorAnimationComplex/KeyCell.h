// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

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

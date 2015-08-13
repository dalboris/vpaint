// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

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

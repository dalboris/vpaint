// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef OPERATOR_H
#define OPERATOR_H

#include <QSet>

namespace VectorAnimationComplex
{

class VAC;
class Cell;

class Operator
{
public:
    // Constructor and Destructor
    Operator();
    virtual ~Operator();

    // Perform the operation now, from a valid state to a valid state
    void now();

private:
    // apply the operator
    virtual void operate()=0;

protected:
    // Set trust mode
    void trustMe();
    void dontTrustMe();

    // Operating on VAC
    //void insertCell(VAC * vac, Cell * c);
    //void removeCell(VAC * vac, Cell * c);

    // Operating on Cell
    void setVAC(Cell * c, VAC * g);
    void setID(Cell * c, int id);

    // Operating on Vertex
    //void setX(Vertex * v, double x);
    //void setY(Vertex * v, double y);

    // Operating on Edge
    //void setLeft(Edge * e, Vertex * left);
    //void setRight(Edge * e, Vertex * right);

private:
    // Operators are single-use only
    bool alreadyPerformed_;

    // trust mode: if true, modified entities are not tracked
    bool trusted_;

    // modified entities (void if not a root operator)
    void modify(Cell * c);
    void modify(VAC * vac);
    QSet<VAC*> modifiedVAC_;
    QSet<Cell*> modifiedCells_;

    // check the validity
    bool check();
};

}

#endif // OPERATOR_H

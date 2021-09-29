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

#ifndef OPERATOR_H
#define OPERATOR_H

#include <QSet>
#include "VAC/vpaint_global.h"

namespace VectorAnimationComplex
{

class VAC;
class Cell;

class Q_VPAINT_EXPORT Operator
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

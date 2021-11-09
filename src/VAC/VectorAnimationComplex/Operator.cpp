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

#include "Operator.h"

#include "VAC.h"
#include "Cell.h"

#include <QtDebug>

namespace VectorAnimationComplex
{

Operator::Operator() :
    alreadyPerformed_(false), trusted_(false)
{
}

Operator::~Operator()
{
}

void Operator::now()
{
    // pre-check
    if(alreadyPerformed_)
    {
        qDebug() << "Trying to perform an operation already performed: abort.";
        return;
    }

    // customizable behaviour (virtual function)
    operate();

    // post-check
    if(!trusted_)
        check();
}

// Set trust mode
void Operator::trustMe() { trusted_ = true; }
void Operator::dontTrustMe() { trusted_ = false; }
/*
// Operating on VAC
void Operator::insertCell(VAC * vac, Cell * c) { modify(c); modify(g); g->insertCell(c); }
void Operator::removeCell(VAC * vac, Cell * c) { modify(c); modify(g); g->removeCell(c); }
*/
// Operating on Cell
void Operator::setVAC(Cell * c, VAC * vac) { modify(c); modify(vac); c->vac_ = vac; }
void Operator::setID(Cell * c, int id) { modify(c); c->id_ = id; }
/*
// Operating on Vertex
void Operator::setX(Vertex * v, double x) { modify(v); v->x_ = x; }
void Operator::setY(Vertex * v, double y) { modify(v); v->y_ = y; }

// Operating on Edge
void Operator::setLeft(Edge * e, Vertex * left) { modify(e); modify(left); modify(e->left_); e->left_ = left; }
void Operator::setRight(Edge * e, Vertex * right) { modify(e); modify(right); modify(e->right_); e->right_ = right; }
*/
// Modify Entities
void Operator::modify(Cell * c)
{
    if(!trusted_)
        /*root_->*/modifiedCells_ << c;
}
void Operator::modify(VAC * vac)
{
    if(!trusted_)
        /*root_->*/modifiedVAC_ << vac;
}

// check the validity
bool Operator::check()
{
    for(VAC * vac: /*root_->*/modifiedVAC_)
        if(!vac->check())
        {
            qDebug() << "A VAC modified by the operator is not valid anymore.";
            return false;
        }
    /*root_->*/modifiedVAC_.clear();

    for(Cell * c: /*root_->*/modifiedCells_)
        if(!c->check())
        {
            qDebug() << "Cell(" << c->id() << ") modified by the operator is not valid anymore.";
            return false;
        }
    /*root_->*/modifiedCells_.clear();

    return true;
}

}

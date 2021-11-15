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

#include "AnimatedCycle.h"

#include "KeyVertex.h"
#include "KeyEdge.h"
#include "InbetweenVertex.h"
#include "InbetweenEdge.h"
#include "EdgeGeometry.h"
#include "VAC.h"

#include <QStack>
#include <QMap>
#include <assert.h>
#include <QtDebug>

namespace VectorAnimationComplex
{

AnimatedCycleNode::AnimatedCycleNode(Cell * cell) :
    cell_(cell),
    previous_(0),
    next_(0),
    before_(0),
    after_(0),
    side_(true)
{
}

// Type
AnimatedCycleNode::NodeType AnimatedCycleNode::nodeType() const
{
    if(cell_->toKeyVertex())
        return KeyVertexNode;
    else if(cell_->toInbetweenVertex())
        return InbetweenVertexNode;
    else if(cell_->toKeyEdge())
    {
        if(cell_->toKeyEdge()->isClosed())
            return KeyClosedEdgeNode;
        else
            return KeyOpenEdgeNode;
    }
    else if(cell_->toInbetweenEdge())
    {
        if(cell_->toInbetweenEdge()->isClosed())
            return InbetweenClosedEdgeNode;
        else
            return InbetweenOpenEdgeNode;
    }
    else
        return InvalidNode;
}
AnimatedCycleNode::CycleType AnimatedCycleNode::cycleType(Time time) const
{
    assert(cell()->exists(time));
    CycleType res = InvalidCycle;
    switch(nodeType())
    {
    case InvalidNode:
        res = InvalidCycle;
        break;
    case KeyVertexNode:
    case InbetweenVertexNode:
        if(next())
        {
            if(next()->cell() == cell())
                res = SteinerCycle;
            else
                res = NonSimpleCycle;
        }
        else
        {
            qWarning("Warning: invalid animated cycle");
            res = InvalidCycle;
        }
        break;
    case KeyClosedEdgeNode:
    case InbetweenClosedEdgeNode:
        res = SimpleCycle;
        break;
    case KeyOpenEdgeNode:
    case InbetweenOpenEdgeNode:
        res = NonSimpleCycle;
        break;
    }
    return res;
}

// Setters
void AnimatedCycleNode::setCell(Cell * cell)
{
    cell_ = cell;
}
void AnimatedCycleNode::setPrevious(AnimatedCycleNode * node)
{
    previous_ = node;
}
void AnimatedCycleNode::setNext(AnimatedCycleNode * node)
{
    next_ = node;
}
void AnimatedCycleNode::setBefore(AnimatedCycleNode * node)
{
    before_ = node;
}
void AnimatedCycleNode::setAfter(AnimatedCycleNode * node)
{
    after_ = node;
}

// Getters
Cell * AnimatedCycleNode::cell() const
{
    return cell_;
}
AnimatedCycleNode * AnimatedCycleNode::previous() const
{
    return previous_;
}
AnimatedCycleNode * AnimatedCycleNode::next() const
{
    return next_;
}
AnimatedCycleNode * AnimatedCycleNode::before() const
{
    return before_;
}
AnimatedCycleNode * AnimatedCycleNode::after() const
{
    return after_;
}

// Spatial cycling
AnimatedCycleNode * AnimatedCycleNode::previous(Time time) const
{
    assert(cell()->exists(time));
    AnimatedCycleNode * res = 0;
    switch(nodeType())
    {
    case InvalidNode:
        res = 0;
        break;
    case KeyVertexNode:
    case KeyClosedEdgeNode:
    case KeyOpenEdgeNode:
    case InbetweenVertexNode:
    case InbetweenClosedEdgeNode:
        res = previous();
        break;
    case InbetweenOpenEdgeNode:
        res = previous();
        while(!res->cell()->exists(time))
            res = res->before();
        break;
    }
    return res;
}
AnimatedCycleNode * AnimatedCycleNode::next(Time time) const
{
    assert(cell()->exists(time));
    AnimatedCycleNode * res = 0;
    switch(nodeType())
    {
    case InvalidNode:
        res = 0; // Redundant, but make clear that this is what I want to return
        break;
    case KeyVertexNode:
    case KeyClosedEdgeNode:
    case KeyOpenEdgeNode:
    case InbetweenVertexNode:
    case InbetweenClosedEdgeNode:
        res = next();
        break;
    case InbetweenOpenEdgeNode:
        res = next();
        if(!res)
        {
            res = 0; // Redundant, but make clear that this is what I want to return
            qWarning("Warning: no next, cycle is invalid");
        }
        else
        {
            while(res && !res->cell()->exists(time))
            {
                res = res->after();
                if(!res)
                {
                    res = 0; // Redundant, but make clear that this is what I want to return
                    qWarning("Warning: no after, cycle is invalid");
                }
            }
        }
        break;
    }
    return res;
}

bool AnimatedCycleNode::side() const
{
    return side_;
}

void AnimatedCycleNode::setSide(bool side)
{
    side_ = side;
}

AnimatedCycle::AnimatedCycle() :
    first_(0)
{
}

AnimatedCycle::AnimatedCycle(AnimatedCycleNode * first) :
    first_(first)
{
}

AnimatedCycle::AnimatedCycle(const AnimatedCycle & other) :
    first_(0)
{
    copyFrom(other);
}

AnimatedCycle & AnimatedCycle::operator=(const AnimatedCycle & other)
{
    if(this != &other)
        copyFrom(other);

    return *this;
}

AnimatedCycle::~AnimatedCycle()
{
    clear();
}

void AnimatedCycle::clear()
{
    const auto& nodeList = nodes();
    for(AnimatedCycleNode * node: nodeList)
        delete node;

    first_ = 0;
}

void AnimatedCycle::copyFrom(const AnimatedCycle & other)
{
    clear();

    QMap<AnimatedCycleNode *, AnimatedCycleNode *> oldToNew;
    oldToNew[0] = 0;
    std::for_each(std::begin(other.nodes())
                  , std::end(other.nodes())
                  , [&](AnimatedCycleNode * oldNode)
    {
        oldToNew[oldNode] = new AnimatedCycleNode(oldNode->cell());
    });

    std::for_each(std::begin(other.nodes())
                  , std::end(other.nodes())
                  , [&](AnimatedCycleNode * oldNode)
    {
        AnimatedCycleNode * newNode = oldToNew[oldNode];
        newNode->setPrevious(oldToNew[oldNode->previous()]);
        newNode->setNext(oldToNew[oldNode->next()]);
        newNode->setBefore(oldToNew[oldNode->before()]);
        newNode->setAfter(oldToNew[oldNode->after()]);
        newNode->setSide(oldNode->side());
    });

    first_ = oldToNew[other.first_];
    tempNodes_ = other.tempNodes_;
}

AnimatedCycleNode  * AnimatedCycle::first() const
{
    return first_;
}

void AnimatedCycle::setFirst(AnimatedCycleNode  * node)
{
    first_ = node;
}

AnimatedCycleNode  * AnimatedCycle::getNode(Time time)
{
    AnimatedCycleNode  * res = first_;
    if(!res)
    {
        qWarning("node(t) not found: no first node");
        return 0;
    }


    while(!res->cell()->exists(time))
    {
        res = res->after();
        if(!res)
        {
            qWarning("node(t) not found: no after node");
            return 0;
        }
    }

    return res;
}

QSet<AnimatedCycleNode*> AnimatedCycle::nodes() const
{
    QSet<AnimatedCycleNode*> res;

    if(!first_)
        return res;

    QStack<AnimatedCycleNode*> toProcess;
    toProcess.push(first_);
    res << first_;
    while(!toProcess.isEmpty())
    {
        AnimatedCycleNode * node = toProcess.pop();
        AnimatedCycleNode * pointedNodes[4] = { node->previous(),
                                                node->next(),
                                                node->before(),
                                                node->after() };
        for(int i=0; i<4; ++i)
        {
            if(pointedNodes[i] && !res.contains(pointedNodes[i]))
            {
                toProcess.push(pointedNodes[i]);
                res << pointedNodes[i];
            }
        }
    }
    return res;
}

CellSet AnimatedCycle::cells() const
{
    CellSet res;
    const auto& cycleNodes = nodes();
    for(AnimatedCycleNode * node: cycleNodes)
        res << node->cell();
    return res;
}
KeyCellSet AnimatedCycle::beforeCells() const
{
    KeyCellSet res;
    const auto& cycleNodes = nodes();
    for(AnimatedCycleNode * node: cycleNodes)
    {
        if(!node->before())
            res.unite(node->cell()->beforeCells());
    }
    return res;
}
KeyCellSet AnimatedCycle::afterCells() const
{
    KeyCellSet res;
    const auto& cycleNodes = nodes();
    for(AnimatedCycleNode * node: cycleNodes)
    {
        if(!node->after())
            res.unite(node->cell()->afterCells());
    }
    return res;
}


// Replace pointed vertex
void AnimatedCycle::replaceVertex(KeyVertex * oldVertex, KeyVertex * newVertex)
{
    const auto& cycleNodes = nodes();
    for(AnimatedCycleNode * node: cycleNodes)
    {
        if(node->cell()->toKeyVertex() == oldVertex)
            node->setCell(newVertex);
    }
}
void AnimatedCycle::replaceHalfedge(const KeyHalfedge & oldHalfedge, const KeyHalfedge & newHalfedge)
{
    const auto& cycleNodes = nodes();
    for(AnimatedCycleNode * node: cycleNodes)
    {
        if(node->cell()->toKeyEdge() == oldHalfedge.edge)
        {
            node->setCell(newHalfedge.edge);
            node->setSide((node->side() == oldHalfedge.side) == newHalfedge.side);
        }
    }
}
void AnimatedCycle::replaceEdges(KeyEdge * oldEdge, const KeyEdgeList & newEdges)
{
    if(oldEdge->isClosed())
    {
        // Get the old key closed edge nodes, sorted
        QList<AnimatedCycleNode *> oldEdgeNodes;
        AnimatedCycleNode * oldEdgeNodeFirst = *getNodes(oldEdge).begin();
        AnimatedCycleNode * oldEdgeNode = oldEdgeNodeFirst;
        do
        {
            oldEdgeNodes << oldEdgeNode;
            oldEdgeNode = oldEdgeNode->next();
        }
        while(oldEdgeNode != oldEdgeNodeFirst);
        int nOldEdgeNodes = oldEdgeNodes.size();

        // Get before inbetween closed edge nodes
        QList<AnimatedCycleNode *> beforeEdgeNodes;
        AnimatedCycleNode * beforeEdgeNodesFirst = oldEdgeNodeFirst->before();
        AnimatedCycleNode * beforeEdgeNode = beforeEdgeNodesFirst;
        do
        {
            beforeEdgeNodes << beforeEdgeNode;
            beforeEdgeNode = beforeEdgeNode->next();
        }
        while(beforeEdgeNode != beforeEdgeNodesFirst);
        int nBeforeEdgeNodes = beforeEdgeNodes.size();
        int beforeInbetweenKeyRatio = nOldEdgeNodes / nBeforeEdgeNodes;

        // Get after inbetween closed edge nodes
        QList<AnimatedCycleNode *> afterEdgeNodes;
        AnimatedCycleNode * afterEdgeNodesFirst = oldEdgeNodeFirst->after();
        AnimatedCycleNode * afterEdgeNode = afterEdgeNodesFirst;
        do
        {
            afterEdgeNodes << afterEdgeNode;
            afterEdgeNode = afterEdgeNode->next();
        }
        while(afterEdgeNode != afterEdgeNodesFirst);
        int nAfterEdgeNodes = afterEdgeNodes.size();
        int afterInbetweenKeyRatio = nOldEdgeNodes / nAfterEdgeNodes;

        // Get side and num of new edge (i.e., num of new edgeNode per old edgeNode)
        int n = newEdges.size();
        bool side = oldEdgeNode->side();

        // Create new nodes
        QList<AnimatedCycleNode *> newEdgeNodes;
        QList<AnimatedCycleNode *> newVertexNodes;
        for(int i=0; i<nOldEdgeNodes; ++i)
        {
            for(int j=0; j<n; ++j)
            {
                AnimatedCycleNode * newEdgeNode = new AnimatedCycleNode(newEdges[i]);
                newEdgeNode->setSide(side);
                newEdgeNodes << newEdgeNode;

                AnimatedCycleNode * newVertexNode = new AnimatedCycleNode(newEdges[i]->endVertex());
                newVertexNodes << newVertexNode;
            }
        }
        int m = newEdgeNodes.size(); // == newVertexNodes.size() == nOldEdgeNodes*n

        // Set pointers of new nodes
        int k = 0;
        for(int i=0; i<nOldEdgeNodes; ++i)
        {
            for(int j=0; j<n; ++j)
            {
                newEdgeNodes[k]->setPrevious(newVertexNodes[ (k-1+m) % m ]);
                newEdgeNodes[k]->setNext(newVertexNodes[k]);
                newEdgeNodes[k]->setBefore(oldEdgeNodes[i]->before());
                newEdgeNodes[k]->setAfter(oldEdgeNodes[i]->after());

                newVertexNodes[k]->setPrevious(newEdgeNodes[k]);
                newVertexNodes[k]->setNext(newEdgeNodes[(k+1) % m ]);
                newVertexNodes[k]->setBefore(oldEdgeNodes[i]->before());
                newVertexNodes[k]->setAfter(oldEdgeNodes[i]->after());

                ++k;
            }
        }

        // Set pointers of before inbetween closed edge nodes
        for(int i=0; i<nBeforeEdgeNodes; ++i)
            beforeEdgeNodes[i]->setAfter(newEdgeNodes[i*beforeInbetweenKeyRatio]);

        // Set pointers of after inbetween closed edge nodes
        for(int i=0; i<nAfterEdgeNodes; ++i)
            afterEdgeNodes[i]->setBefore(newEdgeNodes[i*afterInbetweenKeyRatio]);

        // Update first node. Shouldn't occur, since first_ is supposed to be a inbetween node
        for(int i=0; i<nOldEdgeNodes; ++i)
        {
            if(first_ == oldEdgeNodes[i])
                first_ = newEdgeNodes[0];
        }

        // Delete old nodes
        for(int i=0; i<nOldEdgeNodes; ++i)
            delete oldEdgeNodes[i];
    }
    else
    {
        const auto& oldEdgeNodes = getNodes(oldEdge);
        for(AnimatedCycleNode * oldEdgeNode: oldEdgeNodes)
        {
            int n = newEdges.size();
            bool side = oldEdgeNode->side();

            // Create the new nodes
            QList<AnimatedCycleNode *> newEdgeNodes; // [0..n-1]
            QList<AnimatedCycleNode *> newVertexNodes; // [0..n-2]
            if(side)
            {
                for(int i=0; i<n; ++i)
                {
                    AnimatedCycleNode * newEdgeNode = new AnimatedCycleNode(newEdges[i]);
                    newEdgeNode->setSide(side);
                    newEdgeNodes << newEdgeNode;
                }
                for(int i=0; i<n-1; ++i)
                {
                    AnimatedCycleNode * newVertexNode = new AnimatedCycleNode(newEdges[i]->endVertex());
                    newVertexNodes << newVertexNode;
                }
            }
            else
            {
                for(int i=0; i<n; ++i)
                {
                    AnimatedCycleNode * newEdgeNode = new AnimatedCycleNode(newEdges[n-1-i]);
                    newEdgeNode->setSide(side);
                    newEdgeNodes << newEdgeNode;
                }
                for(int i=0; i<n-1; ++i)
                {
                    AnimatedCycleNode * newVertexNode = new AnimatedCycleNode(newEdges[n-1-i]->startVertex());
                    newVertexNodes << newVertexNode;
                }
            }

            // Set direct pointers
            // previous
            newEdgeNodes[0]->setPrevious(oldEdgeNode->previous());
            for(int i=1; i<n; ++i)
                newEdgeNodes[i]->setPrevious(newVertexNodes[i-1]);
            for(int i=0; i<n-1; ++i)
                newVertexNodes[i]->setPrevious(newEdgeNodes[i]);
            // next
            newEdgeNodes[n-1]->setNext(oldEdgeNode->next());
            for(int i=0; i<n-1; ++i)
                newEdgeNodes[i]->setNext(newVertexNodes[i]);
            for(int i=0; i<n-1; ++i)
                newVertexNodes[i]->setNext(newEdgeNodes[i+1]);
            // before
            for(int i=0; i<n; ++i)
                newEdgeNodes[i]->setBefore(oldEdgeNode->before());
            for(int i=0; i<n-1; ++i)
                newVertexNodes[i]->setBefore(oldEdgeNode->before());
            // after
            for(int i=0; i<n; ++i)
                newEdgeNodes[i]->setAfter(oldEdgeNode->after());
            for(int i=0; i<n-1; ++i)
                newVertexNodes[i]->setAfter(oldEdgeNode->after());

            // Set back pointers
            // previous
            oldEdgeNode->previous()->setNext(newEdgeNodes[0]);
            // next
            oldEdgeNode->next()->setPrevious(newEdgeNodes[n-1]);
            // before
            if(oldEdgeNode->before() && oldEdgeNode->before()->after() == oldEdgeNode)
                oldEdgeNode->before()->setAfter(newEdgeNodes[0]);
            // after
            if(oldEdgeNode->after() && oldEdgeNode->after()->before() == oldEdgeNode)
                oldEdgeNode->after()->setBefore(newEdgeNodes[n-1]);

            // Update first node
            if(first_ == oldEdgeNode) // shouldn't occur, since first_ is supposed to be a inbetween node
                first_ = newEdgeNodes[0];

            // Delete node
            delete oldEdgeNode;
        }
    }
}

QSet<AnimatedCycleNode*>  AnimatedCycle::getNodes(Cell * cell)
{
    QSet<AnimatedCycleNode*> res;
    const auto& cycleNodes = nodes();
    for(AnimatedCycleNode * node: cycleNodes)
    {
        if(node->cell() == cell)
            res << node;
    }
    return res;
}

void AnimatedCycle::replaceInbetweenVertex(InbetweenVertex * sv,
                            InbetweenVertex * sv1, KeyVertex * kv, InbetweenVertex * sv2)
{
    const auto& BetweenVertexNodes = getNodes(sv);
    for(AnimatedCycleNode * nsv: BetweenVertexNodes)
    {
        // Create three new nodes
        AnimatedCycleNode * nsv1 = new AnimatedCycleNode(sv1);
        AnimatedCycleNode * nkv = new AnimatedCycleNode(kv);
        AnimatedCycleNode * nsv2 = new AnimatedCycleNode(sv2);

        if(nsv->next() == nsv) // Steiner cycle case
        {
            // Set direct pointers
            // previous
            nsv1->setPrevious(nsv1);
            nkv->setPrevious(nkv);
            nsv2->setPrevious(nsv2);
            // next
            nsv1->setNext(nsv1);
            nkv->setNext(nkv);
            nsv2->setNext(nsv2);
            // before
            nsv1->setBefore(nsv->before());
            nkv->setBefore(nsv1);
            nsv2->setBefore(nkv);
            // after
            nsv1->setAfter(nkv);
            nkv->setAfter(nsv2);
            nsv2->setAfter(nsv->after());

            // Set back pointers
            // previous
            //   -> no-op
            // next
            //   -> no-op
            // before
            if(nsv->before() && nsv->before()->after() == nsv)
                nsv->before()->setAfter(nsv1);
            // after
            if(nsv->after() && nsv->after()->before() == nsv)
                nsv->after()->setBefore(nsv2);

        }
        else // Non-simple cycle case
        {
            // Set direct pointers
            // previous
            nsv1->setPrevious(nsv->previous());
            nkv->setPrevious(nsv->previous());
            nsv2->setPrevious(nsv->previous());
            // next
            nsv1->setNext(nsv->next());
            nkv->setNext(nsv->next());
            nsv2->setNext(nsv->next());
            // before
            nsv1->setBefore(nsv->before());
            nkv->setBefore(nsv1);
            nsv2->setBefore(nkv);
            // after
            nsv1->setAfter(nkv);
            nkv->setAfter(nsv2);
            nsv2->setAfter(nsv->after());

            // Set back pointers
            // previous
            assert(nsv->previous());
            if(nsv->previous()->next() == nsv)
                nsv->previous()->setNext(nsv1);
            // next
            assert(nsv->next());
            if(nsv->next()->previous() == nsv)
                nsv->next()->setPrevious(nsv2);
            // before
            if(nsv->before() && nsv->before()->after() == nsv)
                nsv->before()->setAfter(nsv1);
            // after
            if(nsv->after() && nsv->after()->before() == nsv)
                nsv->after()->setBefore(nsv2);
        }

        // Update first node
        if(first_ == nsv)
            first_ = nsv1;

        // Delete node
        delete nsv;
    }
}

void AnimatedCycle::replaceInbetweenEdge(InbetweenEdge * se,
                                         InbetweenEdge * se1, KeyEdge * ke, InbetweenEdge * se2)
{
    // Get time
    Time t = ke->time();

    if(se->isClosed())
    {
        // Get old nodes, sorted
        QSet<AnimatedCycleNode*> oldNodesUnsorted = getNodes(se);
        if (oldNodesUnsorted.isEmpty()) {
            // Nothing to do if `se` doesn't belong to this animated cycle
            return;
        }
        AnimatedCycleNode * oldNodeFirst = *oldNodesUnsorted.begin();
        AnimatedCycleNode * oldNode = oldNodeFirst;
        QList<AnimatedCycleNode *> oldNodes;
        do
        {
            oldNodes << oldNode;
            oldNode = oldNode->next();
        }
        while(oldNode != oldNodeFirst);
        int n = oldNodes.size();
        bool side = oldNodeFirst->side();

        // Create new nodes and set orientation
        QList<AnimatedCycleNode *> newNodesBefore;
        QList<AnimatedCycleNode *> newNodesKey;
        QList<AnimatedCycleNode *> newNodesAfter;
        for(int i=0; i<n; ++i)
        {
            AnimatedCycleNode * nse1 = new AnimatedCycleNode(se1);
            AnimatedCycleNode * nke = new AnimatedCycleNode(ke);
            AnimatedCycleNode * nse2 = new AnimatedCycleNode(se2);
            nse1->setSide(side);
            nke->setSide(side);
            nse2->setSide(side);
            newNodesBefore << nse1;
            newNodesKey << nke;
            newNodesAfter << nse2;
        }

        // Set direct pointers
        for(int i=0; i<n; ++i)
        {
            newNodesBefore[i]->setPrevious(newNodesBefore[(i+n-1)%n]);
            newNodesBefore[i]->setNext(newNodesBefore[(i+1)%n]);
            newNodesBefore[i]->setBefore(oldNodes[i]->before());
            newNodesBefore[i]->setAfter(newNodesKey[i]);

            newNodesKey[i]->setPrevious(newNodesKey[(i+n-1)%n]);
            newNodesKey[i]->setNext(newNodesKey[(i+1)%n]);
            newNodesKey[i]->setBefore(newNodesBefore[i]);
            newNodesKey[i]->setAfter(newNodesAfter[i]);

            newNodesAfter[i]->setPrevious(newNodesAfter[(i+n-1)%n]);
            newNodesAfter[i]->setNext(newNodesAfter[(i+1)%n]);
            newNodesAfter[i]->setBefore(newNodesKey[i]);
            newNodesAfter[i]->setAfter(oldNodes[i]->after());
        }

        // Set indirect pointers
        for(int i=0; i<n; ++i)
        {
            AnimatedCycleNode * nse = oldNodes[i];
            AnimatedCycleNode * nse1 = newNodesBefore[i];
            AnimatedCycleNode * nse2 = newNodesAfter[i];

            // before
            AnimatedCycleNode * quasiBeforeNode = nse->before();
            while(quasiBeforeNode && quasiBeforeNode->after() == nse)
            {
                quasiBeforeNode->setAfter(nse1);
                quasiBeforeNode = quasiBeforeNode->previous();
            }

            // after
            AnimatedCycleNode * quasiAfterNode = nse->after();
            while(quasiAfterNode && quasiAfterNode->before() == nse)
            {
                quasiAfterNode->setBefore(nse2);
                quasiAfterNode = quasiAfterNode->next();
            }

            // Update first node
            if(first_ == nse)
                first_ = nse1;
        }

        // Delete nodes
        for(int i=0; i<n; ++i)
            delete oldNodes[i];
    }
    else
    {
        // Perform substitution
        const auto& betweenEdgeNodes = getNodes(se);
        for(AnimatedCycleNode * nse: betweenEdgeNodes)
        {
            // Get boundary nodes
            AnimatedCycleNode * nkvprevious = nse->previous(t);
            AnimatedCycleNode * nkvnext = nse->next(t);
            assert(nkvprevious->cell()->toKeyVertex());
            assert(nkvnext->cell()->toKeyVertex());

            // Create three new nodes
            AnimatedCycleNode * nse1 = new AnimatedCycleNode(se1);
            AnimatedCycleNode * nke = new AnimatedCycleNode(ke);
            AnimatedCycleNode * nse2 = new AnimatedCycleNode(se2);

            // Set orientation
            nse1->setSide(nse->side());
            nke->setSide(nse->side());
            nse2->setSide(nse->side());

            // Set direct pointers
            // previous
            nse1->setPrevious(nkvprevious->before());
            nke->setPrevious(nkvprevious);
            nse2->setPrevious(nse->previous());
            // next
            nse1->setNext(nse->next());
            nke->setNext(nkvnext);
            nse2->setNext(nkvnext->after());
            // before
            nse1->setBefore(nse->before());
            nke->setBefore(nse1);
            nse2->setBefore(nke);
            // after
            nse1->setAfter(nke);
            nke->setAfter(nse2);
            nse2->setAfter(nse->after());

            // Set back pointers
            // previous
            {
                assert(nse->previous());
                AnimatedCycleNode * quasiPreviousNode = nse->previous();
                AnimatedCycleNode * nextOfQuasiPreviousNode = nse2;
                while(quasiPreviousNode && quasiPreviousNode->next() == nse)
                {
                    if(quasiPreviousNode == nkvprevious)
                    {
                        nextOfQuasiPreviousNode = nke;
                        quasiPreviousNode->setNext(nextOfQuasiPreviousNode);
                        nextOfQuasiPreviousNode = nse1;
                    }
                    else
                    {
                        quasiPreviousNode->setNext(nextOfQuasiPreviousNode);
                    }
                    quasiPreviousNode = quasiPreviousNode->before();
                }
            }
            // next
            {
                assert(nse->next());
                AnimatedCycleNode * quasiNextNode = nse->next();
                AnimatedCycleNode * previousOfQuasiNextNode = nse1;
                while(quasiNextNode && quasiNextNode->previous() == nse)
                {
                    if(quasiNextNode == nkvnext)
                    {
                        previousOfQuasiNextNode = nke;
                        quasiNextNode->setPrevious(previousOfQuasiNextNode);
                        previousOfQuasiNextNode = nse2;
                    }
                    else
                    {
                        quasiNextNode->setPrevious(previousOfQuasiNextNode);
                    }
                    quasiNextNode = quasiNextNode->after();
                }
            }
            // before
            {
                AnimatedCycleNode * quasiBeforeNode = nse->before();
                while(quasiBeforeNode && quasiBeforeNode->after() == nse)
                {
                    quasiBeforeNode->setAfter(nse1);
                    quasiBeforeNode = quasiBeforeNode->previous();
                }
            }
            // after
            {
                AnimatedCycleNode * quasiAfterNode = nse->after();
                while(quasiAfterNode && quasiAfterNode->before() == nse)
                {
                    quasiAfterNode->setBefore(nse2);
                    quasiAfterNode = quasiAfterNode->next();
                }
            }

            // Update first node
            if(first_ == nse)
                first_ = nse1;

            // Delete node
            delete nse;
        }
    }
}


// Geometry
void AnimatedCycle::sample(Time time, QList<Eigen::Vector2d> & out)
{
    // A robust sampling scheme. Do not assume that the cycle is valid.
    // More specifically, the


    out.clear();
    AnimatedCycleNode * node = getNode(time);
    if(!node)
    {
        qWarning("Warning: sampling failed: no node found");
        return;
    }

    AnimatedCycleNode::CycleType cycleType = node->cycleType(time);

    if(cycleType == AnimatedCycleNode::NonSimpleCycle)
    {
        AnimatedCycleNode * firstOpenHalfedge = node;
        if(firstOpenHalfedge->nodeType() == AnimatedCycleNode::KeyVertexNode ||
           firstOpenHalfedge->nodeType() == AnimatedCycleNode::InbetweenVertexNode)
        {
            firstOpenHalfedge = firstOpenHalfedge->next(time);
            if(!firstOpenHalfedge)
            {
                qWarning("Warning: sampling (partially) failed: no next node found");
                return;
            }
        }
        //assert(firstOpenHalfedge->nodeType() == AnimatedCycleNode::KeyOpenEdgeNode ||
        //       firstOpenHalfedge->nodeType() == AnimatedCycleNode::InbetweenOpenEdgeNode);
        if( !(firstOpenHalfedge->nodeType() == AnimatedCycleNode::KeyOpenEdgeNode ||
               firstOpenHalfedge->nodeType() == AnimatedCycleNode::InbetweenOpenEdgeNode))
        {
            qWarning("Warning: sampling (partially) failed: wrong node type");
            return;
        }

        AnimatedCycleNode * openHalfedge = firstOpenHalfedge;
        do
        {
            //assert(openHalfedge->nodeType() == AnimatedCycleNode::KeyOpenEdgeNode ||
            //       openHalfedge->nodeType() == AnimatedCycleNode::InbetweenOpenEdgeNode);
            if(!(openHalfedge->nodeType() == AnimatedCycleNode::KeyOpenEdgeNode ||
                   openHalfedge->nodeType() == AnimatedCycleNode::InbetweenOpenEdgeNode))
            {
                qWarning("Warning: sampling (partially) failed: wrong node type");
                return;
            }

            if(openHalfedge->nodeType() == AnimatedCycleNode::KeyOpenEdgeNode)
            {
                QList<Eigen::Vector2d> sampling = openHalfedge->cell()->toKeyEdge()->geometry()->sampling();
                if(openHalfedge->side())
                    for(int i=0; i<sampling.size()-1; ++i) // -1 because we don't want to duplicate last sample
                        out << sampling[i];
                else
                    for(int i=sampling.size()-1; i>0; --i) // -1 because we don't want to duplicate last sample
                        out << sampling[i];
            }
            else if(openHalfedge->nodeType() == AnimatedCycleNode::InbetweenOpenEdgeNode)
            {
                QList<Eigen::Vector2d> sampling = openHalfedge->cell()->toInbetweenEdge()->getGeometry(time);
                if(openHalfedge->side())
                    for(int i=0; i<sampling.size()-1; ++i) // -1 because we don't want to duplicate last sample
                        out << sampling[i];
                else
                    for(int i=sampling.size()-1; i>0; --i) // -1 because we don't want to duplicate last sample
                        out << sampling[i];
            }

            // Go next two times, i.e.:
            //  openHalfedge = openHalfedge->next(time)->next(time);
            // but with safety checks
            openHalfedge = openHalfedge->next(time);
            if(!openHalfedge)
            {
                qWarning("Warning: sampling (partially) failed: no next node found");
                return;
            }
            openHalfedge = openHalfedge->next(time);
            if(!openHalfedge)
            {
                qWarning("Warning: sampling (partially) failed: no next node found");
                return;
            }
        }
        while(openHalfedge != firstOpenHalfedge);
    }
    else if(cycleType == AnimatedCycleNode::SimpleCycle)
    {
        //assert(node->nodeType() == AnimatedCycleNode::KeyClosedEdgeNode ||
        //       node->nodeType() == AnimatedCycleNode::InbetweenClosedEdgeNode);
        if(!(node->nodeType() == AnimatedCycleNode::KeyClosedEdgeNode ||
               node->nodeType() == AnimatedCycleNode::InbetweenClosedEdgeNode))
        {
            qWarning("Warning: sampling (partially) failed: wrong node type");
            return;
        }

        AnimatedCycleNode * firstClosedHalfedge = node;
        AnimatedCycleNode * closedHalfedge = node;

        if(node->nodeType() == AnimatedCycleNode::KeyClosedEdgeNode)
        {
            do
            {
                //assert(closedHalfedge->nodeType() == AnimatedCycleNode::KeyClosedEdgeNode);
                if(closedHalfedge->nodeType() != AnimatedCycleNode::KeyClosedEdgeNode)
                {
                    qWarning("Warning: sampling (partially) failed: wrong node type");
                    return;
                }
                QList<Eigen::Vector2d> sampling = closedHalfedge->cell()->toKeyEdge()->geometry()->sampling();
                if(closedHalfedge->side())
                    for(int i=0; i<sampling.size()-1; ++i) // -1 because we don't want to duplicate last sample
                        out << sampling[i];
                else
                    for(int i=sampling.size()-1; i>0; --i) // -1 because we don't want to duplicate last sample
                        out << sampling[i];

                closedHalfedge = closedHalfedge->next(time);
                if(!closedHalfedge)
                {
                    qWarning("Warning: sampling (partially) failed: no next node found");
                    return;
                }
            }
            while(closedHalfedge != firstClosedHalfedge);
        }
        else // node->nodeType() == AnimatedCycleNode::InbetweenClosedEdgeNode
        {
            do
            {
                //assert(closedHalfedge->nodeType() == AnimatedCycleNode::InbetweenClosedEdgeNode);
                if(closedHalfedge->nodeType() != AnimatedCycleNode::InbetweenClosedEdgeNode)
                {
                    qWarning("Warning: sampling (partially) failed: wrong node type");
                    return;
                }
                QList<Eigen::Vector2d> sampling = closedHalfedge->cell()->toInbetweenEdge()->getGeometry(time);
                if(closedHalfedge->side())
                    for(int i=0; i<sampling.size()-1; ++i) // -1 because we don't want to duplicate last sample
                        out << sampling[i];
                else
                    for(int i=sampling.size()-1; i>0; --i) // -1 because we don't want to duplicate last sample
                        out << sampling[i];

                closedHalfedge = closedHalfedge->next(time);
                if(!closedHalfedge)
                {
                    qWarning("Warning: sampling (partially) failed: no next node found");
                    return;
                }
            }
            while(closedHalfedge != firstClosedHalfedge);
        }
    }
    else if(cycleType == AnimatedCycleNode::SteinerCycle)
    {
        //assert(node->nodeType() == AnimatedCycleNode::KeyVertexNode ||
        //       node->nodeType() == AnimatedCycleNode::InbetweenVertexNode);
        if(!(node->nodeType() == AnimatedCycleNode::KeyVertexNode ||
               node->nodeType() == AnimatedCycleNode::InbetweenVertexNode))
        {
            qWarning("Warning: sampling (partially) failed: wrong node type");
            return;
        }

        if(node->nodeType() == AnimatedCycleNode::KeyVertexNode)
        {
            out << node->cell()->toKeyVertex()->pos();
        }
        else
        {
            out << node->cell()->toInbetweenVertex()->pos(time);
        }
    }
    else
    {
        qWarning("Warning: sampling failed: invalid cycle");
        return;
        //assert(false && "invalid cycle");
    }
}


void AnimatedCycle::remapPointers(VAC * newVAC)
{
    const auto& cycleNodes = nodes();
    for(AnimatedCycleNode * node: cycleNodes)
    {
        node->setCell(newVAC->getCell(node->cell()->id()));
    }
}

void AnimatedCycle::convertTempIdsToPointers(VAC * vac)
{
    int n = tempNodes_.size();

    // First pass, create all nodes
    QList<AnimatedCycleNode*> nodes;
    for(int i=0; i<n; ++i)
    {
        TempNode tempNode = tempNodes_[i];
        AnimatedCycleNode * node = new AnimatedCycleNode(vac->getCell(tempNode.cell));
        node->setSide(tempNodes_[i].side);
        nodes << node;
    }

    // Second pass, link nodes together
    for(int i=0; i<n; ++i)
    {
        nodes[i]->setPrevious(nodes[tempNodes_[i].previous]);
        nodes[i]->setNext(nodes[tempNodes_[i].next]);
        if(tempNodes_[i].before == -1)
            nodes[i]->setBefore(0);
        else
            nodes[i]->setBefore(nodes[tempNodes_[i].before]);
        if(tempNodes_[i].after == -1)
            nodes[i]->setAfter(0);
        else
            nodes[i]->setAfter(nodes[tempNodes_[i].after]);
    }

    // Find first
    if (n > 0) {
        first_ = nodes[0];
        while(first_->before())
            first_ = first_->before();
    }

    // Clean
    tempNodes_.clear();
}

namespace
{
QString str(int i)
{
    return QString().setNum(i);
}

QString str(const QMap<AnimatedCycleNode*,int> & nodeMap, AnimatedCycleNode * node)
{
    if(node)
        return str(nodeMap[node]);
    else
        return "_";
}

int toInt(const QMap<int,int> & map, const QString & str)
{
    if(str == "" || str == "_")
        return -1;
    else
        return map[str.toInt()];
}
}

QString AnimatedCycle::toString() const
{
    // Create correspondence between node pointers and [1..n] where n is the number of nodes
    QMap<AnimatedCycleNode*,int> nodeMap;
    int id = 1;
    std::for_each(std::begin(nodes())
                  , std::end(nodes())
                  , [&](AnimatedCycleNode * node)
    {
        nodeMap[node] = id++;
    });

    // Write
    QString res;
    res += "[";
    bool first = true;
    std::for_each(std::begin(nodes())
                  , std::end(nodes())
                  , [&](AnimatedCycleNode * node)
    {
        if(first)
            first = false;
        else
            res += " ";

        // Node id
        res += str(nodeMap[node]) + ":(";

        // Referenced cell id [and side, for edges]
        res += str(node->cell()->id());
        if(node->cell()->toEdgeCell())
            res += ( node->side() ? "+" : "-");
        res += ",";

        // Previous/Next/Before/After node pointers
        res += str(nodeMap, node->previous()) + ",";
        res += str(nodeMap, node->next()) + ",";
        res += str(nodeMap, node->before()) + ",";
        res += str(nodeMap, node->after()) + ")";
    });
    res += "]";

    return res;
}

void AnimatedCycle::fromString(const QString & str)
{
    clear();
    tempNodes_.clear();

    // Split the string into substrings corresponding to the node data
    // Example:
    //  "[1:(15+,2,5,_,_) 2:(12,1,2,3,4)]" becomes:
    //  [ "1" ; "15+" ; "2" ; "5" ; "_" ; "_" ; "2" ; "12" ; "1" ; "2" ; "3" ; "4" ]
    QStringList d = str.split(QRegExp("[\\[\\]\\s\\,\\(\\):]"), QString::SkipEmptyParts); // use , ( ) [ ] : and whitespaces as delimiters

    // Get the number of nodes
    int n = d.size()/6;

    // Create a map between saved node id and node id in [0..n-1],
    // since we will save this data into an array, and discard the "saved node id"
    QMap<int,int> map;
    for(int i=0; i<n; ++i)
        map[ d[6*i].toInt() ] = i;

    // Store data in tempNodes
    for(int i=0; i<n; ++i)
    {
        AnimatedCycle::TempNode tempNode;

        // Referenced cell and side
        QString cellside = d[6*i+1];
        int l = cellside.length();
        QChar side = cellside.at(l-1);
        QString cell = cellside.left(l-1);
        if(side == '+' || side == '-')
        {
            tempNode.cell = cell.toInt();
            tempNode.side = (side == '+') ? true : false;
        }
        else
        {
            tempNode.cell = cellside.toInt();
            tempNode.side = true; // true or false is irrelevant, choose true arbitrarily
        }

        // Previous/Next/Before/After node pointers
        tempNode.previous = toInt(map, d[6*i+2]);
        tempNode.next = toInt(map, d[6*i+3]);
        tempNode.before = toInt(map, d[6*i+4]);
        tempNode.after = toInt(map, d[6*i+5]);

        // Add to list of nodes
        tempNodes_ << tempNode;
    }
}



} // end namespace VectorAnimationComplex

QTextStream & operator<<(QTextStream & out, const VectorAnimationComplex::AnimatedCycle & cycle)
{
    using namespace VectorAnimationComplex;

    // Create correspondence between node pointers and [0..n-1] where n is the number of nodes
    QMap<AnimatedCycleNode*,int> nodeMap;
    int id = 0;
    std::for_each(std::begin(cycle.nodes())
                  , std::end(cycle.nodes())
                  , [&](AnimatedCycleNode * node)
    {
        nodeMap[node] = id++;
    });

    // Write to file
    out << "[";
    std::for_each(std::begin(cycle.nodes())
                  , std::end(cycle.nodes())
                  , [&](AnimatedCycleNode * node)
    {
        if(nodeMap[node]!=0) out << " ,";
        out << " "
            << "(" << node->cell()->id()
            << "," << nodeMap[node->previous()]
            << "," << nodeMap[node->next()]
            << "," << (node->before() ? nodeMap[node->before()] : -1)
            << "," << (node->after() ? nodeMap[node->after()] : -1)
            << "," << node->side() << ")";
    });
    out << " ]";

    return out;
}

QTextStream & operator>>(QTextStream & in, VectorAnimationComplex::AnimatedCycle & cycle)
{
    using namespace VectorAnimationComplex;

    cycle.tempNodes_.clear();

    // put the list to read as a string
    QString listAsString;
    in >> listAsString; // read "["
    int openedBracket = 1;
    char c;
    while(openedBracket != 0)
    {
        in >> c;
        if(c == '[')
            openedBracket++;
        if(c == ']')
            openedBracket--;

        listAsString.append(c);
    }

    // test if the list is void
    QString copyString = listAsString;
    QTextStream test(&copyString);
    QString b1, b2;
    test >> b1 >> b2;

    // if not void
    if(b2 != "]")
    {
        QString nuple;
        QTextStream newIn(&listAsString);
        newIn >> b1; // read "["
        QString delimiter(",");
        while(delimiter == ",")
        {
            newIn >> nuple;
            QStringList list = nuple.split(QRegExp("\\s*[\\(\\,\\)]\\s*"),
                                           QString::SkipEmptyParts);
            AnimatedCycle::TempNode tempNode;
            tempNode.cell = list[0].toInt();
            tempNode.previous = list[1].toInt();
            tempNode.next = list[2].toInt();
            tempNode.before = list[3].toInt();
            tempNode.after = list[4].toInt();
            tempNode.side = list[5].toInt();
            cycle.tempNodes_ << tempNode;

            newIn >> delimiter;
        }
    }

    return in;
}

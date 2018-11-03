// Copyright (C) 2012-2018 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Layer.h"

#include "Background/Background.h"
#include "VectorAnimationComplex/VAC.h"

Layer::Layer(NoInit_)
{

}

Layer::Layer() :
    background_(new Background(this)),
    vac_(new VectorAnimationComplex::VAC())
{
    connect(background_, SIGNAL(changed()), this, SIGNAL(changed()));
    connect(background_, SIGNAL(checkpoint()), this, SIGNAL(checkpoint()));

    connect(vac_, SIGNAL(changed()), this, SIGNAL(changed()));
    connect(vac_, SIGNAL(checkpoint()), this, SIGNAL(checkpoint()));
    connect(vac_, SIGNAL(needUpdatePicking()), this, SIGNAL(needUpdatePicking()));
    connect(vac_, SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()));
}

Layer::~Layer()
{
    delete vac_;
}

Layer * Layer::clone()
{
    Layer * res = new Layer(NoInit_());
    res->background_ = new Background(background());
    res->vac_ = vac_->clone();
    return res;
}

QString Layer::stringType()
{
    return "Layer";
}

void Layer::draw(Time time, ViewSettings & viewSettings)
{
    // Draw the VAC only. Drawing the background is handled by View.
    vac()->draw(time, viewSettings);
}

void Layer::drawPick(Time time, ViewSettings & viewSettings)
{
    vac()->drawPick(time, viewSettings);
}

void Layer::setHoveredObject(Time time, int id)
{
    vac()->setHoveredObject(time, id);
}

void Layer::setNoHoveredObject()
{
    vac()->setNoHoveredObject();
}

void Layer::select(Time time, int id)
{
    vac()->select(time, id);
}

void Layer::deselect(Time time, int id)
{
    vac()->deselect(time, id);
}

void Layer::toggle(Time time, int id)
{
    vac()->toggle(time, id);
}

void Layer::deselectAll(Time time)
{
    vac()->deselectAll(time);
}

void Layer::deselectAll()
{
    vac()->deselectAll();
}

void Layer::invertSelection()
{
    vac()->invertSelection();
}

void Layer::exportSVG_(Time t, QTextStream & out)
{
    // This function does not export the background, because the API
    // for Background::exportSVG() requires the canvas' size, which
    // we don't know here.
    //
    // Therefore, it is the responsibility of clients to manually call
    // background()->exportSVG beforehand if they wish.
    //
    // XXX We should improve this ugly design
    //
    vac()->exportSVG(t, out);
}

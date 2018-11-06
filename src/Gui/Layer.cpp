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
#include "XmlStreamReader.h"
#include "XmlStreamWriter.h"

Layer::Layer(NoInit_)
{

}

Layer::Layer(const QString & layerName) :
    background_(new Background(this)),
    vac_(new VectorAnimationComplex::VAC()),
    name_(layerName),
    isVisible_(true)
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
    res->background_ = new Background(*background());
    res->vac_ = vac_->clone();
    res->name_ = name_;
    res->isVisible_ = isVisible_;
    return res;

    // Note:
    //
    // When I first implemented this function, I wrote:
    //
    //   Background(background())
    //
    // instead of
    //
    //   Background(*background())
    //
    // This caused a crash because it called the Background(QObject*)
    // constructor, not the copy constructor as I intended. In retrospect, the
    // "clone()" idiom seems safer and better suited for pointer-like objects
    // with identity, even though using the copy constructor seemed like
    // more idiomatic C++.
}

QString Layer::stringType()
{
    return "Layer";
}

void Layer::draw(Time time, ViewSettings & viewSettings)
{
    // Draw the VAC only. Drawing the background is handled by View.
    if (isVisible()) {
        vac()->draw(time, viewSettings);
    }
}

void Layer::drawPick(Time time, ViewSettings & viewSettings)
{
    if (isVisible()) {
        vac()->drawPick(time, viewSettings);
    }
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

void Layer::read(XmlStreamReader & xml)
{
    while (xml.readNextStartElement())
    {
        if (xml.name() == "background")
        {
            background()->read(xml);
        }
        else if (xml.name() == "objects")
        {
            vac()->read(xml);
        }
        else
        {
            xml.skipCurrentElement();
        }
    }

    emit needUpdatePicking();
    emit changed();
    emit selectionChanged();
}

void Layer::write(XmlStreamWriter & xml)
{
    // Background
    xml.writeStartElement("background");
    background()->write(xml);
    xml.writeEndElement();

    // Vector animation complex
    xml.writeStartElement("objects");
    vac()->write(xml);
    xml.writeEndElement();
}

QString Layer::name() const
{
    return name_;
}

void Layer::setName(const QString & newName)
{
    if (newName != name_)
    {
        name_ = newName;
        emit layerAttributesChanged();
    }
}

bool Layer::isVisible() const
{
    return isVisible_;
}

void Layer::setVisible(bool b)
{
    if (b != isVisible_)
    {
        isVisible_ = b;
        emit layerAttributesChanged();
    }
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

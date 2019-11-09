// Copyright (C) 2012-2019 The VPaint Developers..
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

void Layer::init_(
        Background * background,
        VectorAnimationComplex::VAC * vac,
        const QString & layerName,
        bool isVisible)
{
    background_ = background;
    vac_ = vac;
    name_ = layerName;
    isVisible_ = isVisible;

    connect(background_, SIGNAL(changed()), this, SIGNAL(changed()));
    connect(background_, SIGNAL(checkpoint()), this, SIGNAL(checkpoint()));

    connect(vac_, SIGNAL(changed()), this, SIGNAL(changed()));
    connect(vac_, SIGNAL(checkpoint()), this, SIGNAL(checkpoint()));
    connect(vac_, SIGNAL(needUpdatePicking()), this, SIGNAL(needUpdatePicking()));
    connect(vac_, SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()));
}

Layer::Layer(const QString & layerName)
{
    init_(new Background(this),
          new VectorAnimationComplex::VAC(),
          layerName,
          true);
}

Layer::~Layer()
{
    delete vac_;
}

Layer * Layer::clone()
{
    // Two previous attempts at implementing this resulted
    // in bugs. The first is because I called
    //     new Background(background())
    // instead of
    //     new Background(*background())
    // And the second because new Background(this) was called
    // in a constructor before "this" was properly initialized,
    // that is, parent-child relationship were messed up.
    // The conclusion is that using copy-constructor is much
    // trickier than the "clone" idiom, which should be preferred
    // based on this experience: it is much less bug-prone when dealing
    // with pointer-like objects with identity.
    //

    Layer * res = new Layer(NoInit_());
    res->init_(new Background(*background(), res),
               vac_->clone(),
               name_,
               isVisible_);
    return res;
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
    // Name
    name_ = "Layer";
    if(xml.attributes().hasAttribute("name"))
    {
        name_ = xml.attributes().value("name").toString();
    }

    // Visible
    isVisible_ = true;
    if(xml.attributes().hasAttribute("visible"))
    {
        QString value = xml.attributes().value("visible").toString();
        if (value == "true")
        {
            isVisible_ = true;
        }
        else if (value == "false")
        {
            isVisible_ = false;
        }
    }

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
    emit layerAttributesChanged();
}

void Layer::write(XmlStreamWriter & xml)
{
    // Name
    xml.writeAttribute("name", name());

    // Visible
    xml.writeAttribute("visible", isVisible() ? "true" : "false");

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
        emit changed();
        emit needUpdatePicking();
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

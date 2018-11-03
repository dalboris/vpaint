// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef LAYER_H
#define LAYER_H

#include "SceneObject.h"

class Background;
namespace VectorAnimationComplex { class VAC; }

class Layer : public SceneObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Layer)

private:
    // Private constructor with no initialization of its members.
    struct NoInit_{};
    Layer(NoInit_);

public:
    Layer();
    ~Layer() override;

    Layer * clone() override;
    QString stringType() override;
    
    void draw(Time time, ViewSettings & viewSettings) override;
    void drawPick(Time time, ViewSettings & viewSettings) override;

    void setHoveredObject(Time time, int id) override;
    void setNoHoveredObject() override;
    void select(Time time, int id) override;
    void deselect(Time time, int id) override;
    void toggle(Time time, int id) override;
    void deselectAll(Time time) override;
    void deselectAll() override;
    void invertSelection() override;

    Background * background() const { return background_; }
    VectorAnimationComplex::VAC * vac() const { return vac_; }

signals:
    void changed();
    void checkpoint();
    void needUpdatePicking();
    void selectionChanged();

protected:
    void exportSVG_(Time t, QTextStream & out) override;

private:
    Background * background_;
    VectorAnimationComplex::VAC * vac_;
};

#endif

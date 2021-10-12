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

#ifndef LAYER_H
#define LAYER_H

#include "SceneObject.h"
#include "VAC/vpaint_global.h"

class Background;
namespace VectorAnimationComplex { class VAC; }
class XmlStreamReader;
class XmlStreamWriter;

class Q_VPAINT_EXPORT Layer : public SceneObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Layer)

private:
    // Private constructor with no initialization of its members.
    struct NoInit_{};
    Layer(NoInit_);

    // Takes ownership of passed pointers
    void init_(Background * background,
               VectorAnimationComplex::VAC * vac,
               const QString & layerName,
               bool isVisible);

public:
    explicit Layer(const QString & layerName = QStringLiteral("Layer"));
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

    void read(XmlStreamReader & xml);
    void write(XmlStreamWriter & xml);

    Background * background() const { return background_; }
    VectorAnimationComplex::VAC * vac() const { return vac_; }

    QString name() const;
    void setName(const QString & name);

    bool isVisible() const;
    void setVisible(bool b);

signals:
    // defined in base class (Scene Object)
    // void changed();
    // void checkpoint();
    // void needUpdatePicking();

    void selectionChanged();
    void layerAttributesChanged();

protected:
    void exportSVG_(Time t, QTextStream & out) override;

private:
    Background * background_;
    VectorAnimationComplex::VAC * vac_;
    QString name_;
    bool isVisible_;
};

#endif

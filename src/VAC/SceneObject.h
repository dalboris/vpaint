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

#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

#include <QObject>
#include <QString>

#include "ExportSettings.h"
#include "Picking.h"
#include "TimeDef.h"
#include "ViewSettings.h"

class QTextStream;
class QToolBar;

class SceneObject: public QObject
{
    Q_OBJECT

public:
    SceneObject() : 
        canBeSaved_(false), 
        shouldBeSaved_(false) {}

    virtual ~SceneObject() {}
    virtual SceneObject * clone() {return new SceneObject();}
    virtual QString stringType() {return "SceneObject";}
    
    virtual void draw(Time /*time*/, ViewSettings & /*viewSettings*/) {}
    virtual void drawPick(Time /*time*/, ViewSettings & /*viewSettings*/) {}

    // Selecting and Highlighting
    virtual void setHoveredObject(Time /*time*/, int /*id*/) {}
    virtual void setNoHoveredObject() {}
    virtual void select(Time /*time*/, int /*id*/) {}
    virtual void deselect(Time /*time*/, int /*id*/) {}
    virtual void toggle(Time /*time*/, int /*id*/) {}
    virtual void deselectAll(Time /*time*/) {}
    virtual void deselectAll() {}
    virtual void invertSelection() {}

    // Save property
    bool canBeSaved() { return canBeSaved_; }
    bool shouldBeSaved() { return shouldBeSaved_; }
    void setShouldBeSaved(bool b) { shouldBeSaved_ = b; }
    void save(QTextStream & out);
    void exportSVG(QTextStream & out, const VectorExportSettings & settings, Time t);
    static SceneObject * read(QTextStream & in);


    // GUI
    //virtual void populateToolBar(QToolBar * /*toolBar*/) {}

signals:
    void changed();    // can be emitted for any reason, even a change of color.
                       // if called several times in a row, the view will still
                       // be redrawn only once.

    void checkpoint(); // when emitted, the whole scene is copied and put in the
                       // undo list. Make sure to call this only once, when necessary

    void needUpdatePicking(); // Make sure to call this only once, when necessary

    
protected:
    virtual void save_(QTextStream & out);
    virtual void exportSVG_(QTextStream & out, const VectorExportSettings & settings, Time t);
    bool canBeSaved_;

private:
    bool shouldBeSaved_;
};



#endif

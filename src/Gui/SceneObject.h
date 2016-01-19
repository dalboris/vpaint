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

#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

#include <QObject>
#include <QString>
#include "TimeDef.h" 
#include "Picking.h" 
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
    void exportSVG(Time t, QTextStream & out);
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
    virtual void exportSVG_(Time t, QTextStream & out);
    bool canBeSaved_;

private:
    bool shouldBeSaved_;
};



#endif

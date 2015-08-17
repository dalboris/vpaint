// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef PICKING_H
#define PICKING_H

typedef unsigned int uint;
typedef unsigned char uchar;

class Picking
{
public:
    // ---- Interface for scene objects ----
    
    static void glColor(uint id); 
    //
    // Could be also named setId(uint innerId).
    // 
    // This  name  reminds  the   user  of  the  underlying  picking
    // mechanism,  and  then  to remove  all  OpenGL::glColor(color)
    // calls that would cause the picking to fail.
    //
    // A proper  way to do would  be to have a  customized class for
    // drawing,   like   Drawing::setColor(),   Drawing::drawLine(),
    // removing  the SceneObject  dependancy to  OpenGL,  and making
    // possible  to automagically  ignore  Drawing::setColor() calls
    // when in  picking mode. I  had no time  to do this,  and using
    // directly OpenGL in SceneObject makes it more flexible.
    //
    // Basically, to convert a draw() method into a drawPick():
    //   - don't draw objects that can't be selected
    //   - remove all the OpenGL::glColor(color) calls
    //   - add the Picking::glColor(id) calls

    
    // ------   Interface for view/scene ------

    // set
    static void setTime(uint time); // for view
    static void setIndex(uint index);// for scene
    
    // get
    class Object 
    {
    public:
        // Constructor: the null object by default
        Object(uint t=-1, uint i=-1, uint id=-1);
        
        // test
        bool operator==(const Object & other) const;
        bool isNull() const { return *this == Object(); }

        // getter
        uint time() const { return time_; }
        uint index() const { return index_; }
        uint id() const { return id_; }
        
    private:
        uint time_;
        uint index_;
        uint id_;
    };
    static Object objectFromRGB(uchar r, uchar g, uchar b);

    
private:
    // 
    // Mapping between RGBA value and picked object:
    //
    // rgba_ = RRRR RRRR GGGG GGGG BBBB BBBB AAAA AAAA
    //         ^\_________/\_______________/ \_______/
    //         | index (9)      id (14)       255 (8) 
    //         |
    //      time (1)
    //    
    static uint rgba_;
};

#endif

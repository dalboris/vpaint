// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "Picking.h"
#include "OpenGL.h"
#include <QtDebug>

uint Picking::rgba_ = 0x000000FF;

/*********************************************************************
 *                      OBJECT -> RGBA
 *
 * At each frame drawing,  called once per selectable object, possibly
 * millions of them.
 * 
 * => Efficiency is critical
 * 
 */

void Picking::setTime(uint time)
{
    // clear the existing value
    rgba_ &= 0x7FFFFFFF;

    // clamp time, set it
    rgba_ += (time & 0x1) << 31; 
}

void Picking::setIndex(uint index)
{
    // clear the existing value
    rgba_ &= 0x803FFFFF;

    // clamp time, set it
    rgba_ += (index & 0x1FF) << 22;
}

void Picking::glColor(uint id)
{
    // clear the existing value
    rgba_ &= 0xFFC000FF;

    // clamp time, set it
    rgba_ += (id & 0x3FFF) << 8;    
    
    // call OpenGL
    // -> fails because of endianness
    //glColor4ubv(reinterpret_cast<GLubyte*>(&rgba_)); 
    
    GLubyte color[4];
    color[0] = (rgba_ & 0xFF000000) >> 24;
    color[1] = (rgba_ & 0x00FF0000) >> 16;
    color[2] = (rgba_ & 0x0000FF00) >> 8;
    color[3] = 255;
    glColor4ubv(color);
}

/*********************************************************************
 *                      RGBA -> OBJECT
 *
 * Called only once per MouseMove. Efficiency not critical.
 *
 */ 

Picking::Object Picking::objectFromRGB(uchar r, uchar g, uchar b)
{
    uint R = r;
    uint G = g;
    uint B = b;
    
    uint time = R >> 7;
    uint index = ((R & 0x7F) << 2) + (G >> 6);
    uint id = ((G & 0x3F) << 8) + B;

    return Object(time, index, id);
}

bool Picking::Object::operator==(const Picking::Object & other) const
{
    return 
        (time_==other.time_) && 
        (index_==other.index_) &&
        (id_==other.id_); 
} 

Picking::Object::Object(uint t, uint i, uint id) :
    time_(t),
    index_(i),
    id_(id)
{
} 

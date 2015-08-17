// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef KEYFRAME_H
#define KEYFRAME_H


// ----------- KeyFrame -----------------
    
class KeyFrame
{
public:
    KeyFrame(int f=0) :
        frame(f), previous(0), next(0) {} 

    int frame;
    KeyFrame *previous;
    KeyFrame *next;
};

class KeyFrameDouble: public KeyFrame
{
public:
    KeyFrameDouble(int f=0, double d=0) :
        KeyFrame(f), value(d) {}
    
    double value;
};




// ----------- KeyFrame Line -----------------

class AnimatedData
{
public:
    AnimatedData();
    
    KeyFrame *first;
    KeyFrame *last;
    KeyFrame *current;
    
    int currentFrame;
    
    virtual void updateCurrent(int frame);
    void insertKeyFrame(KeyFrame *k);
};

class AnimatedDouble: public AnimatedData
{
public:
    // idea:  should  I provide  a  method to  get  the  value at  a
    // specific time, but without modifying the timeline position?
    //void  updateCurrent(int  currentFrame);  //  can be  added  to
    // eventually precompute things.
    void createKeyFrame(int frame, double d);
    
    double value(int frame);

private:
    //double currentValue_; can be used for precomputation
};


#endif

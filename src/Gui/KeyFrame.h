// Copyright (C) 2012-2019 The VPaint Developers
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

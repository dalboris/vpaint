// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "KeyFrame.h"
#include "Timeline.h"

// ----------- KeyFrame Line -----------------

AnimatedData::AnimatedData() :
    first(0), last(0), current(0)
{
    // TODO: find some  way to have the correct  frame when creating
    // the animated data. Here, we assume it is created at frame 0

    // now, it's possible thanks to the static currentFrame() method
    // be  careful,  it  means  there  can't  be  2  timelines  with
    // different currentFrame in the software
    
    currentFrame = 0;//Timeline::currentFrame();
}

    
void AnimatedData::updateCurrent(int frame)
{
    // ideally this operation should be the faster for, in order:
    //   - frame = currentFrame+1
    //   - frame = currentFrame-1
    //   - frame ~ currentFrame
    //   - frame = playingWindowFirstFrame
    //   - frame = playingWindowLastFrame
    //   - arbitrary position
    //
    // It would  be nice  to modify it  for arbitrary  position, for
    // instance using a table to  access in constant time a keyframe
    // located at a specific "N second" window. (for large animation)
    //
    // A simpler idea  is also to find the  closest keyframe between
    // first, current and last,  and search from that starting point
    // (instead of always using current except when out of bound)

    if(currentFrame == frame)
        return;

    currentFrame = frame;
    
    // at most one keyframe
    if(first == last)
        return;
    
    // at least two keyframes
    if(frame <= first->frame)
        current = first;
    else if(frame >= last->frame)
        current = last;
    else
    {
        while(current->frame < frame)
            current = current->next;
        while(current->frame > frame)
            current = current->previous;    
    }
}

void AnimatedData::insertKeyFrame(KeyFrame *k)
{
    // configuration 1: there exists no keyframe yet
    if(first == 0)
    {
        k->previous = 0;
        k->next = 0;

        first = k;
        last = k;
        current = k;
    }

    // configuration 2: there exists one and only one keyframe
    else if(first == last)
    {
        // the new keyframe should replace the existed one
        if(k->frame == first->frame)
        {
            delete first;

            // back to configuration 1
            k->previous = 0;
            k->next = 0;

            first = k;
            last = k;
            current = k;
        }
        else // there exists now exactly two keyframes
        {
            if(k->frame < first->frame) 
            {
                k->previous = 0;
                k->next = first;
                k->next->previous = k;
                first = k;
            }
            else //(k->frame > first->frame) 
            {
                k->previous = first;
                k->next = 0;
                k->previous->next = k;
                last = k;
            }
            
            if(currentFrame < last->frame)
                current = first;
            else
                current = last;
        }
    }

    // configuration 3: there exists already at least 2 keyframes
    else
    {
        // first, deals with extreme cases
        if(k->frame < first->frame)
        {
            k->previous = 0;
            k->next = first;
            k->next->previous = k;

            if(currentFrame < first->frame)
                current = k;
               
            first = k;
        }
        else if(k->frame == first->frame)
        {
            k->previous = 0;
            k->next = first->next;
            k->next->previous = k;

            if(current == first)
                current = k;
            
            delete first;
            first = k;
        }
        else if(k->frame == last->frame)
        {
            k->previous = last->previous;
            k->next = 0;
            k->previous->next = k;

            if(current == last)
                current = k;
            
            delete last;
            last = k;
        }
        else if(k->frame > last->frame)
        {
            k->previous = last;
            k->next = 0;
            k->previous->next = k;

            if(currentFrame >= k->frame)
                current = k;
            
            last = k;
        }
        
        
        else // first < k < last
        {
            // only one should be fine
            KeyFrame *beforeOrEqualK;
            if(currentFrame <= first->frame)
                beforeOrEqualK = first;
            else if(currentFrame >= last->frame)
                beforeOrEqualK = last;
            else
                beforeOrEqualK = current;

            while(beforeOrEqualK->frame < k->frame)
                beforeOrEqualK = beforeOrEqualK->next;
            while(beforeOrEqualK->frame > k->frame)
                beforeOrEqualK = beforeOrEqualK->previous;

            // Equal: replace the keyframe
            if(beforeOrEqualK->frame == k->frame)
            {
                k->previous = beforeOrEqualK->previous;
                k->next = beforeOrEqualK->next;
                k->next->previous = k;
                k->previous->next = k;

                if(current == beforeOrEqualK)
                    current = k;
                
                delete beforeOrEqualK;
            }
            else // Before: insert the keyframe
            {
                k->previous = beforeOrEqualK;
                k->next = beforeOrEqualK->next;
                k->next->previous = k;
                k->previous->next = k;

                if(current == beforeOrEqualK && currentFrame >= k->frame)
                    current = k;
            }
        }
    }        
}

void AnimatedDouble::createKeyFrame(int frame, double d)
{
    KeyFrameDouble *k = new KeyFrameDouble(frame, d);
    insertKeyFrame(k);
}
    
double AnimatedDouble::value(int frame)
{
    // TODO: avoid doing this everytime (well, maybe not...)
    currentFrame = frame;
    updateCurrent(frame);
    
    // configuration 1: no keyframe -> return default value
    if(!first)
        return 0;

    // configuration 2: one keyframe -> return this value
    else if(first == last)
        return static_cast<KeyFrameDouble*>(first)->value;

    // configuration 3-1: before first frame -> return first
    else if(currentFrame <= first->frame)
        return static_cast<KeyFrameDouble*>(first)->value;

    // configuration 3-2: after last frame -> return last
    else if(currentFrame >= last->frame)
        return static_cast<KeyFrameDouble*>(last)->value;

    // configuration 3-3: at a specific keyframe -> return it
    else if(currentFrame == current->frame)
        return static_cast<KeyFrameDouble*>(current)->value;

    // configuration 3-4: strictly between two frame -> interpolate
    else
    {
        // by now, linear interpolation
        int t1 = current->frame;
        double d1 = static_cast<KeyFrameDouble*>(current)->value;
        
        int t2 = current->next->frame;
        double d2 = static_cast<KeyFrameDouble*>(current->next)->value;
        
        double u = (double)(currentFrame - t1) / (double)(t2 - t1);
        
        return d1 + u*(d2-d1);
    }    
}

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

#include <QTextStream>
#include <cmath>

#include "TimeDef.h"

#define FPS 1
#define EPSILON 1.0e-10


Time::Time() :
    type_(ExactFrame),
    frame_(0), time_(0)
{
}

Time::Time(int f) : 
    type_(ExactFrame), 
    frame_(f),
    time_(f/(double)FPS) 
{
}

Time::Time(int f, bool justAfter) : 
    frame_(f),
    time_(f/(double)FPS)
{
    if(justAfter)
    {
        type_ = JustAfterFrame;
        // TODO:  change the  implementation, use  the nextafter()
        // methods, providing in C99 or C++11
        // e.g.: 1.0e7 + 1.0e-10 != 1.0e7
        // but 1.0e5 + 1.0e-10 >> 1.0e5 (>> means lot of floats inbetween)
        
        // would still be ok in normal cases, and even -1 day < t < 1 day...
        time_ += EPSILON;
    }
    else
    {
        type_ = JustBeforeFrame;
        time_ -= EPSILON;
    }
}

Time::Time(double t) : 
    type_(FloatTime), 
    frame_(std::floor(t*FPS)), // by default, truncate towards negative infinite
    time_(t) 
{
    double eps = 1.0e-4;
    double rounded = std::floor(t*FPS+0.5);
    double rest = t - rounded;
    if( -eps<rest && rest<eps ) // unless epsilon-close to an integer (e.g., t=41.9999)
    {
        type_ = ExactFrame;     // in which case we assume the frame is exact
        frame_ = (int) rounded; // and truncate to nearest (e.g., 42)
    }
}

bool Time::operator<(const Time & other) const
{
    if(type() != other.type())
    {
        // a robust way would not do that, but ok in normal cases
        return floatTime() < other.floatTime();
    }
    else
    {
        switch (type())
        {
        case ExactFrame:
        case JustBeforeFrame:
        case JustAfterFrame:
            return frame() < other.frame();
        case FloatTime:
            return floatTime() < other.floatTime();
        default:
            return false;
        }
    }
}

bool Time::operator>(const Time & other) const
{
    return other<*this;
}

bool Time::operator>=(const Time & other) const
{
    return (*this > other) || (*this == other);
}

bool Time::operator<=(const Time & other) const
{
    return (*this < other) || (*this == other);
}

bool Time::operator==(const Time & other) const
{
    if(type() != other.type())
        return false;
    else
    {
        switch (type())
        {
        case ExactFrame:
        case JustBeforeFrame:
        case JustAfterFrame:
            return frame() == other.frame();
        case FloatTime:
            return floatTime() == other.floatTime();
        default:
            return true;
        }
    }
}
    
bool Time::operator!=(const Time & other) const
{
    return !( *this == other );
}

Time Time::operator+(const Time & other) const
{
    if(type() != other.type())
    {
        // a robust way would not do that, but ok in normal cases
        return Time(floatTime() + other.floatTime());
    }
    else
    {
        switch (type())
        {
        case ExactFrame:
        case JustBeforeFrame:
        case JustAfterFrame:
            return Time(frame() + other.frame());
        case FloatTime:
            return Time(floatTime() + other.floatTime());
        default:
            return Time();
        }
    }
}

Time Time::operator-(const Time & other) const
{
    if(type() != other.type())
    {
        // a robust way would not do that, but ok in normal cases
        return Time(floatTime() - other.floatTime());
    }
    else
    {
        switch (type())
        {
        case ExactFrame:
        case JustBeforeFrame:
        case JustAfterFrame:
            return Time(frame() - other.frame());
        case FloatTime:
            return Time(floatTime() - other.floatTime());
        default:
            return Time();
        }
    }
}


void Time::save(QTextStream & out)
{
    switch (type())
    {
    case ExactFrame:
        out << "ExactFrame " << frame();
        return;
    case JustBeforeFrame:
        out << "JustBeforeFrame " << frame();
        return;
    case JustAfterFrame:
        out << "JustAfterFrame " << frame();
        return;
    case FloatTime:
        out << "FloatTime " << floatTime();
        return;
    default:
        return;
    }    
}

QTextStream & operator<<(QTextStream & str, const Time & time)
{
    switch (time.type())
    {
    case Time::ExactFrame:
        str << "ExactFrame " << time.frame();
        return str;
    case Time::JustBeforeFrame:
        str << "JustBeforeFrame " << time.frame();
        return str;
    case Time::JustAfterFrame:
        str << "JustAfterFrame " << time.frame();
        return str;
    case Time::FloatTime:
        str << "FloatTime " << time.floatTime();
        return str;
    default:
        return str;
    }
}

QTextStream & operator>>(QTextStream & str, Time & time)
{
    QString type;
    str >> type;

    if(type == "ExactFrame")
    {
        int frame;
        str >> frame;
        time = Time(frame);
    }
    else if(type == "JustBeforeFrame")
    {
        int frame;
        str >> frame;
        time = Time(frame, 0);
    }
    else if(type == "JustAfterFrame")
    {
        int frame;
        str >> frame;
        time = Time(frame, 1);
    }
    else if(type == "FloatTime")
    {
        double t;
        str >> t;
        time = Time(t);
    }

    return str;
}

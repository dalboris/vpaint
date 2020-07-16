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

#ifndef TIME_H
#define TIME_H

class QTextStream;

class Time
{
public:

    enum Type {
        ExactFrame,
        JustBeforeFrame,
        JustAfterFrame,
        FloatTime
    };

    
    Time(); // exact frame 0
    
    Time(int f); // exact frame f
    
    Time(int f, bool justAfter); // just before or after frame f
    
    Time(double t); // floating value, automatically converted to ExactFrame
                    // if close enough within small threshold

    virtual ~Time() {} // make the constructor virtual
    
    Type type() const { return type_; }
    int frame() const { return frame_; }
    double floatTime() const { return time_; } // Assume 1 fps, i.e. ExactFrame(5) < FloatTime(5.2) < ExactFrame(6).

    bool operator<(const Time & other) const;
    bool operator<=(const Time & other) const;
    bool operator>(const Time & other) const;
    bool operator>=(const Time & other) const;
    bool operator==(const Time & other) const;
    bool operator!=(const Time & other) const;
    
    Time operator+(const Time & other) const;
    Time operator-(const Time & other) const;

    // Save and Load
    virtual void save(QTextStream & out);
    friend QTextStream & operator<<(QTextStream &, const Time & time);
    friend QTextStream & operator>>(QTextStream &, Time & time);

private:
    Type type_;
    int frame_;
    double time_;
};

#endif

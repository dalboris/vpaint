// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

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

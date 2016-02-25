// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_FRAME_H
#define OPENVAC_FRAME_H

#include <cmath>

namespace OpenVAC
{

/// \class Frame Core/Frame.h
/// \brief A class representing a frame of an animation as a double.
///
/// Even though represented as a double, a Frame will typically be an "integer"
/// such as 1.0, 2.0, 3.0, etc. Here is the typical way frames are initialized:
///
/// \code
/// Frame f1 = 1;
/// Frame f2 = 2;
/// Frame f3; // == 0
/// \endcode
///
/// A subframe can be represented as a non-integer double, such as 1.5:
///
/// \code
/// Frame f4 = 1.5;
/// \endcode
///
/// To test whether a Frame is an integer frame or a subframe, use isInteger().
///
/// \code
/// f1.isInteger(); // True:  f1 is an integer frame
/// f4.isInteger(); // False: f4 is a subframe
/// \endcode
///
/// The method isSubframe() is provided for convenience, which give the
/// opposite boolean of isInteger().
///
/// \code
/// f1.isSubframe(); // False: f1 is an integer frame
/// f4.isSubframe(); // True:  f4 is a subframe
/// \endcode
///
/// You can compare frames together (Are they equal? Which one is smaller?),
/// which is a safe double comparison "within epsilon", so that integer
/// frames are always equal to the closest exact integer, even in the case
/// or numerical errors. Ints and doubles are implicitely casted to Frame, so
/// you can use a very convenient syntax:
///
/// \code
/// if (f1 == 1) { ... }
/// if (f1 < f2) { ... }
/// if (f1 > 0) { ... }
/// \endcode
///
/// Frames can be added/substracted together, frames can be added/substracted
/// with an int or double, and frames can be multiplied/divided by an int or
/// double. However, frames can't be multiplied/divided together. The operation
/// is always interpreted in a 'double' sense.
///
/// \code
/// Frame midFrame = (f1+f2)/2; // == 1.5
/// f1 = f2 + 5; // == 7
/// f1 = 2*f2; // == 4
/// f1 += 2; // == 6
/// f1++; // f1 is now 7; 6 is returned
/// ++f1; // f1 is now 8; 8 is returned
/// ++midFrame; // midFrame is now 2.5; 2.5 is returned
/// \endcode
///
/// Sometimes, for instance to compute interpolated geometry, it may be
/// useful to get the underlying double representing the frame:
///
/// \code
/// double frameAsDouble = f1.toDouble();
/// \endcode
///
/// However, most of the time, you should use the Frame class directly instead
/// of manipulating doubles. Indeed, even though any 32-bit int can be exactly
/// represented as a double, numerical errors can occur when they come from
/// a computation, such as:
///
/// \code
/// int n = 10;
/// double df = 1.0 / n;
/// double frame = 0;
/// for (int i=0; i<n; ++i)
///     frame += df;
///
/// assert(frame == 1.0);                // FAIL
/// assert(frame == 0.9999999999999999); // PASS
/// \endcode
///
/// The Frame class takes care of this for you by reimplementing the comparison
/// operators to compare 'within epsilon', such that we have the following:
///
/// \code
/// int n = 10;
/// double df = 1.0 / n;
/// Frame frame = 0;
/// for (int i=0; i<n; ++i)
///     frame += df;
///
/// assert(frame.isInteger()); // PASS
/// assert(frame == 1.0);      // PASS
/// assert(frame == 1);        // PASS
/// \endcode
///
/// It is possible to take the floor, ceil, and round of a frame:
///
/// \code
/// Frame f18 = 1.8;
/// Frame::floor(f18); // returns Frame(1)
/// Frame::ceil(f18);  // returns Frame(2)
/// Frame::round(f2);  // returns Frame(2)
/// \endcode
///
/// If f.isInteger() is true, then we are guaranteed to have:
///
/// \code
/// Frame::floor(f).toDouble() == Frame::ceil(f).toDouble(); // == Frame::round(f).toDouble()
/// \endcode
///
/// If f.isSubframe() is true, then we are guaranteed to have:
///
/// \code
/// Frame::floor(f).toDouble() + 1.0 == Frame::ceil(f).toDouble();
/// \endcode
///
/// Note that Frame::floor() and std::floor() may disagree, by design. Indeed,
/// when a frame is epsilon-close to an integer N (i.e., when f.isInteger()
/// returns true), then Frame::floor() returns N no matter if f.toDouble() is
/// smaller or bigger than N. The same is of true for Frame::ceil() and
/// std::ceil(). See the examples below for clarification:
///
/// \code
/// double justBelowOne = 0.9999999999999999;
/// double justAboveOne = 1.0000000000000002;
///
/// Frame::floor(justBelowOne); // == 1.0;
/// Frame::ceil(justBelowOne);  // == 1.0;
/// std::floor(justBelowOne);   // == 0.0;
/// std::ceil(justBelowOne);    // == 1.0;
///
/// Frame::floor(justAboveOne); // == 1.0;
/// Frame::ceil(justAboveOne);  // == 1.0;
/// std::floor(justAboveOne);   // == 1.0;
/// std::ceil(justAboveOne);    // == 2.0;
/// \endcode
///
/// To cast the Frame to an int, call the static method Frame::toInt(f), which
/// is equivalent to (int)Frame::floor(f).toDouble(). If you want to round or
/// ceil instead of floor, you can use Frame::round(f).toInt() or
/// Frame::ceil(f).toInt().
///
/// Frames are FPS-independent, they are not aware of time. To convert a
/// frame to a time in seconds, use toSeconds(double fps), or conversely
/// to get a frame from a time in seconds, use fromSeconds(double fps).

class Frame
{
private:
    static const double EPS;

public:
    // Constructors
    Frame()             : value_(0.0)   {}
    Frame(double value) : value_(value) {}

    // Comparison operators
    friend bool operator==(const Frame & f1, const Frame & f2);
    friend bool operator!=(const Frame & f1, const Frame & f2);
    friend bool operator< (const Frame & f1, const Frame & f2);
    friend bool operator> (const Frame & f1, const Frame & f2);
    friend bool operator<=(const Frame & f1, const Frame & f2);
    friend bool operator>=(const Frame & f1, const Frame & f2);

    // Arithmetic operators
    friend Frame operator+(const Frame & f1, const Frame & f2);
    friend Frame operator-(const Frame & f1, const Frame & f2);
    friend Frame operator*(double scalar, const Frame & f);
    friend Frame operator*(const Frame & f, double scalar);
    friend Frame operator/(const Frame & f, double scalar);

    // Compound assignment operators
    friend Frame & operator+=(Frame & f1, const Frame & f2);
    friend Frame & operator-=(Frame & f1, const Frame & f2);
    friend Frame & operator*=(Frame & f, double scalar);
    friend Frame & operator/=(Frame & f, double scalar);

    // Increment and decrement
    friend Frame & operator++(Frame & f);
    friend Frame & operator--(Frame & f);
    friend Frame   operator++(Frame & f, int);
    friend Frame   operator--(Frame & f, int);

    // Floor, ceil, and round
    static Frame floor(const Frame & f) { return Frame(std::floor(f.value_ + EPS)); }
    static Frame ceil (const Frame & f) { return Frame(std::ceil (f.value_ - EPS)); }
    static Frame round(const Frame & f) { return Frame(std::floor(f.value_ + 0.5)); }

    // Test whether the frame is an integer frame or a subframe
    bool isInteger()  const { return floor(*this).value_ == ceil(*this).value_; }
    bool isSubframe() const { return floor(*this).value_ != ceil(*this).value_; }

    // Convert to double and int
    double toDouble() const { return value_; }
    int    toInt   () const { return Frame::floor(*this).value_; }

    // Convert from and to time in seconds
    double toSeconds(double fps) const { return value_ / fps; }
    static Frame fromSeconds(double t, double fps) { return Frame(t * fps); }
    
private:
    double value_;
};

// Comparison operators
inline bool operator==(const Frame & f1, const Frame & f2) { return std::abs(f1.value_ - f2.value_) <= Frame::EPS; }
inline bool operator!=(const Frame & f1, const Frame & f2) { return !(f1 == f2); }
inline bool operator< (const Frame & f1, const Frame & f2) { return (f1 != f2) && (f1.value_ < f2.value_); }
inline bool operator> (const Frame & f1, const Frame & f2) { return (f1 != f2) && (f1.value_ > f2.value_); }
inline bool operator<=(const Frame & f1, const Frame & f2) { return (f1 == f2) || (f1.value_ < f2.value_); }
inline bool operator>=(const Frame & f1, const Frame & f2) { return (f1 == f2) || (f1.value_ > f2.value_); }

// Arithmetic operators
inline Frame operator+(const Frame & f1, const Frame & f2) { return Frame(f1.value_ + f2.value_); }
inline Frame operator-(const Frame & f1, const Frame & f2) { return Frame(f1.value_ - f2.value_); }
inline Frame operator*(double scalar, const Frame & f)     { return Frame(scalar * f.value_); }
inline Frame operator*(const Frame & f, double scalar)     { return Frame(scalar * f.value_); }
inline Frame operator/(const Frame & f, double scalar)     { return Frame(f.value_ / scalar); }

// Compound assignment operators
inline Frame & operator+=(Frame & f1, const Frame & f2) { f1.value_ += f2.value_; return f1; }
inline Frame & operator-=(Frame & f1, const Frame & f2) { f1.value_ -= f2.value_; return f1; }
inline Frame & operator*=(Frame & f, double scalar)     { f.value_  *= scalar;    return f;  }
inline Frame & operator/=(Frame & f, double scalar)     { f.value_  /= scalar;    return f;  }

// Increment and decrement
inline Frame & operator++(Frame & f) { f.value_ += 1.0; return f; }
inline Frame & operator--(Frame & f) { f.value_ -= 1.0; return f; }
inline Frame   operator++(Frame & f, int) { Frame res = f; f.value_ += 1.0; return res; }
inline Frame   operator--(Frame & f, int) { Frame res = f; f.value_ -= 1.0; return res; }

}

#endif // OPENVAC_FRAME_H

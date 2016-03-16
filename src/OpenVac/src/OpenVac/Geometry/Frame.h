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

namespace OpenVac
{

/// \class Frame OpenVac/Geometry/Frame.h
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
///
class Frame
{
private:
    static double EPS() { return 1.0e-10; }

public:

    /**************************** Constructors *******************************/

    ///
    /// Constructs a zero-initialized super Frame.
    ///
    Frame()             : value_(0.0)   {}
    ///
    /// Constructs a Frame initialized by the given \p value.
    ///
    Frame(double value) : value_(value) {}


    /************************** Comparison operators *************************/

    ///
    /// Returns \c true if \p f1 and \p f2 are epsilon-close; otherwise returns
    /// \c false.
    ///
    /// Example:
    ///
    /// \code
    /// Frame one = 1;
    /// Frame two = 2;
    /// Frame justBelowOne = 0.9999999999999999;
    /// Frame justAboveOne = 1.0000000000000002;
    ///
    /// // All asserts below pass
    /// assert(one + two == 3);
    /// assert(justBelowOne == one);
    /// assert(justAboveOne == one);
    /// \endcode
    ///
    friend bool operator==(const Frame & f1, const Frame & f2);
    ///
    /// Returns \c true if \p f1 and \p f2 are distant by more than epsilon;
    /// otherwise returns \c false.
    ///
    friend bool operator!=(const Frame & f1, const Frame & f2);
    ///
    /// Returns \c true if \p f1 and \p f2 are not epsilon-close and \p f1 is
    /// lesser than \p f2; otherwise returns \c false.
    ///
    friend bool operator< (const Frame & f1, const Frame & f2);
    ///
    /// Returns \c true if \p f1 and \p f2 are not epsilon-close and \p f1 is
    /// greater than \p f2; otherwise returns \c false.
    ///
    friend bool operator> (const Frame & f1, const Frame & f2);
    ///
    /// Returns \c true if \p f1 and \p f2 are epsilon-close or \p f1 is
    /// lesser than \p f2; otherwise returns \c false.
    ///
    friend bool operator<=(const Frame & f1, const Frame & f2);
    ///
    /// Returns \c true if \p f1 and \p f2 are epsilon-close or \p f1 is
    /// greater than \p f2; otherwise returns \c false.
    ///
    friend bool operator>=(const Frame & f1, const Frame & f2);


    /************************** Arithmetic operators *************************/

    ///
    /// Returns the sum of \p f1 and \p f2.
    ///
    friend Frame operator+(const Frame & f1, const Frame & f2);
    ///
    /// Returns the difference between \p f1 and \p f2.
    ///
    friend Frame operator-(const Frame & f1, const Frame & f2);
    ///
    /// Returns the multiplication of \p f by \p scalar.
    ///
    friend Frame operator*(double scalar, const Frame & f);
    ///
    /// Returns the multiplication of \p f by \p scalar.
    ///
    friend Frame operator*(const Frame & f, double scalar);
    ///
    /// Returns the division of \p f by \p scalar.
    ///
    friend Frame operator/(const Frame & f, double scalar);


    /*********************** Compound assignment operators *******************/

    ///
    /// Adds \p f2 to \p f1.
    ///
    friend Frame & operator+=(Frame & f1, const Frame & f2);
    ///
    /// Substracts \p f2 from \p f1.
    ///
    friend Frame & operator-=(Frame & f1, const Frame & f2);
    ///
    /// Multiplies \p f by \p scalar.
    ///
    friend Frame & operator*=(Frame & f, double scalar);
    ///
    /// Divides \p f by \p scalar.
    ///
    friend Frame & operator/=(Frame & f, double scalar);


    /************************** Increment and decrement **********************/

    ///
    /// Increments \p f by 1.0. Returns value of \p f after incrementation.
    ///
    friend Frame & operator++(Frame & f);
    ///
    /// Decrements \p f by 1.0. Returns value of \p f after decrementation.
    ///
    friend Frame & operator--(Frame & f);
    ///
    /// Increments \p f by 1.0. Returns value of \p f before incrementation.
    ///
    friend Frame   operator++(Frame & f, int);
    ///
    /// Decrements \p f by 1.0. Returns value of \p f before decrementation.
    ///
    friend Frame   operator--(Frame & f, int);


    /************************** Floor, ceil, and round ***********************/

    ///
    /// Returns the closest exact integer frame if \p f is an integer frame;
    /// otherwise returns the largest exact integer frame not greater than \p f.
    ///
    static Frame floor(const Frame & f) { return Frame(std::floor(f.value_ + EPS())); }
    ///
    /// Returns the closest exact integer frame if \p f is an integer frame;
    /// otherwise returns the smaller exact integer frame not less than \p f.
    ///
    static Frame ceil (const Frame & f) { return Frame(std::ceil (f.value_ - EPS())); }
    ///
    /// Returns the closest exact integer frame of \p f.
    ///
    static Frame round(const Frame & f) { return Frame(std::floor(f.value_ + 0.5)); }


    /***************** Test for integer frame or subframe ********************/

    ///
    /// Returns \c true if \p f is epsilon-close to an exact integer frame;
    /// otherwise returns false.
    ///
    bool isInteger()  const { return floor(*this).value_ == ceil(*this).value_; }
    ///
    /// Returns \c false if \p f is epsilon-close to an exact integer frame;
    /// otherwise returns true.
    ///
    bool isSubframe() const { return floor(*this).value_ != ceil(*this).value_; }


    /************************ Convert to double and int **********************/

    ///
    /// Returns the underlying double representing the frame.
    ///
    double toDouble() const { return value_; }
    ///
    /// Returns Frame::floor(*this) as an int. We remind that Frame::floor(*this)
    /// is always an exact integer frame.
    ///
    int    toInt   () const { return Frame::floor(*this).value_; }


    /************************** Convert from and to time in seconds **********/

    ///
    /// Returns the time in seconds corresponding to this Frame, according to the
    /// given \p fps.
    ///
    double toSeconds(double fps) const { return value_ / fps; }
    ///
    /// Returns the Frame corresponding to the given time in seconds \p t,
    /// according to the given \p fps.
    ///
    static Frame fromSeconds(double t, double fps) { return Frame(t * fps); }
    
private:
    double value_;
};

// Comparison operators
inline bool operator==(const Frame & f1, const Frame & f2) { return std::abs(f1.value_ - f2.value_) <= Frame::EPS(); }
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

} // end namespace OpenVac

#endif // OPENVAC_FRAME_H

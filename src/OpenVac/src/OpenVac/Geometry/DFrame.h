// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_DFRAME_H
#define OPENVAC_DFRAME_H

#include <cmath>

namespace OpenVac
{

/// \class DFrame Geometry/DFrame.h
/// \brief A class representing a frame of an animation as a double.
///
/// Even though represented as a double, a DFrame will typically be an "integer"
/// such as 1.0, 2.0, 3.0, etc. Here is the typical way frames are initialized:
///
/// \code
/// DFrame f1 = 1;
/// DFrame f2 = 2;
/// DFrame f3; // == 0
/// \endcode
///
/// A subframe can be represented as a non-integer double, such as 1.5:
///
/// \code
/// DFrame f4 = 1.5;
/// \endcode
///
/// To test whether a DFrame is an integer frame or a subframe, use isInteger().
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
/// or numerical errors. Ints and doubles are implicitely casted to DFrame, so
/// you can use a very convenient syntax:
///
/// \code
/// if (f1 == 1) { ... }
/// if (f1 < f2) { ... }
/// if (f1 > 0) { ... }
/// \endcode
///
/// DFrames can be added/substracted together, frames can be added/substracted
/// with an int or double, and frames can be multiplied/divided by an int or
/// double. However, frames can't be multiplied/divided together. The operation
/// is always interpreted in a 'double' sense.
///
/// \code
/// DFrame midDFrame = (f1+f2)/2; // == 1.5
/// f1 = f2 + 5; // == 7
/// f1 = 2*f2; // == 4
/// f1 += 2; // == 6
/// f1++; // f1 is now 7; 6 is returned
/// ++f1; // f1 is now 8; 8 is returned
/// ++midDFrame; // midDFrame is now 2.5; 2.5 is returned
/// \endcode
///
/// Sometimes, for instance to compute interpolated geometry, it may be
/// useful to get the underlying double representing the frame:
///
/// \code
/// double frameAsDouble = f1.toDouble();
/// \endcode
///
/// However, most of the time, you should use the DFrame class directly instead
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
/// The DFrame class takes care of this for you by reimplementing the comparison
/// operators to compare 'within epsilon', such that we have the following:
///
/// \code
/// int n = 10;
/// double df = 1.0 / n;
/// DFrame frame = 0;
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
/// DFrame f18 = 1.8;
/// DFrame::floor(f18); // returns DFrame(1)
/// DFrame::ceil(f18);  // returns DFrame(2)
/// DFrame::round(f2);  // returns DFrame(2)
/// \endcode
///
/// If f.isInteger() is true, then we are guaranteed to have:
///
/// \code
/// DFrame::floor(f).toDouble() == DFrame::ceil(f).toDouble(); // == DFrame::round(f).toDouble()
/// \endcode
///
/// If f.isSubframe() is true, then we are guaranteed to have:
///
/// \code
/// DFrame::floor(f).toDouble() + 1.0 == DFrame::ceil(f).toDouble();
/// \endcode
///
/// Note that DFrame::floor() and std::floor() may disagree, by design. Indeed,
/// when a frame is epsilon-close to an integer N (i.e., when f.isInteger()
/// returns true), then DFrame::floor() returns N no matter if f.toDouble() is
/// smaller or bigger than N. The same is of true for DFrame::ceil() and
/// std::ceil(). See the examples below for clarification:
///
/// \code
/// double justBelowOne = 0.9999999999999999;
/// double justAboveOne = 1.0000000000000002;
///
/// DFrame::floor(justBelowOne); // == 1.0;
/// DFrame::ceil(justBelowOne);  // == 1.0;
/// std::floor(justBelowOne);   // == 0.0;
/// std::ceil(justBelowOne);    // == 1.0;
///
/// DFrame::floor(justAboveOne); // == 1.0;
/// DFrame::ceil(justAboveOne);  // == 1.0;
/// std::floor(justAboveOne);   // == 1.0;
/// std::ceil(justAboveOne);    // == 2.0;
/// \endcode
///
/// To cast the DFrame to an int, call the static method DFrame::toInt(f), which
/// is equivalent to (int)DFrame::floor(f).toDouble(). If you want to round or
/// ceil instead of floor, you can use DFrame::round(f).toInt() or
/// DFrame::ceil(f).toInt().
///
/// DFrames are FPS-independent, they are not aware of time. To convert a
/// frame to a time in seconds, use toSeconds(double fps), or conversely
/// to get a frame from a time in seconds, use fromSeconds(double fps).

class DFrame
{
private:
    static double EPS() { return 1.0e-10; }

public:

    /**************************** Constructors *******************************/

    ///
    /// Constructs a zero-initialized super DFrame.
    ///
    DFrame()             : value_(0.0)   {}
    ///
    /// Constructs a DFrame initialized by the given \p value.
    ///
    DFrame(double value) : value_(value) {}


    /************************** Comparison operators *************************/

    ///
    /// Returns \c true if \p f1 and \p f2 are epsilon-close; otherwise returns
    /// \c false.
    ///
    /// Example:
    ///
    /// \code
    /// DFrame one = 1;
    /// DFrame two = 2;
    /// DFrame justBelowOne = 0.9999999999999999;
    /// DFrame justAboveOne = 1.0000000000000002;
    ///
    /// // All asserts below pass
    /// assert(one + two == 3);
    /// assert(justBelowOne == one);
    /// assert(justAboveOne == one);
    /// \endcode
    ///
    friend bool operator==(const DFrame & f1, const DFrame & f2);
    ///
    /// Returns \c true if \p f1 and \p f2 are distant by more than epsilon;
    /// otherwise returns \c false.
    ///
    friend bool operator!=(const DFrame & f1, const DFrame & f2);
    ///
    /// Returns \c true if \p f1 and \p f2 are not epsilon-close and \p f1 is
    /// lesser than \p f2; otherwise returns \c false.
    ///
    friend bool operator< (const DFrame & f1, const DFrame & f2);
    ///
    /// Returns \c true if \p f1 and \p f2 are not epsilon-close and \p f1 is
    /// greater than \p f2; otherwise returns \c false.
    ///
    friend bool operator> (const DFrame & f1, const DFrame & f2);
    ///
    /// Returns \c true if \p f1 and \p f2 are epsilon-close or \p f1 is
    /// lesser than \p f2; otherwise returns \c false.
    ///
    friend bool operator<=(const DFrame & f1, const DFrame & f2);
    ///
    /// Returns \c true if \p f1 and \p f2 are epsilon-close or \p f1 is
    /// greater than \p f2; otherwise returns \c false.
    ///
    friend bool operator>=(const DFrame & f1, const DFrame & f2);


    /************************** Arithmetic operators *************************/

    ///
    /// Returns the sum of \p f1 and \p f2.
    ///
    friend DFrame operator+(const DFrame & f1, const DFrame & f2);
    ///
    /// Returns the difference between \p f1 and \p f2.
    ///
    friend DFrame operator-(const DFrame & f1, const DFrame & f2);
    ///
    /// Returns the multiplication of \p f by \p scalar.
    ///
    friend DFrame operator*(double scalar, const DFrame & f);
    ///
    /// Returns the multiplication of \p f by \p scalar.
    ///
    friend DFrame operator*(const DFrame & f, double scalar);
    ///
    /// Returns the division of \p f by \p scalar.
    ///
    friend DFrame operator/(const DFrame & f, double scalar);


    /*********************** Compound assignment operators *******************/

    ///
    /// Adds \p f2 to \p f1.
    ///
    friend DFrame & operator+=(DFrame & f1, const DFrame & f2);
    ///
    /// Substracts \p f2 from \p f1.
    ///
    friend DFrame & operator-=(DFrame & f1, const DFrame & f2);
    ///
    /// Multiplies \p f by \p scalar.
    ///
    friend DFrame & operator*=(DFrame & f, double scalar);
    ///
    /// Divides \p f by \p scalar.
    ///
    friend DFrame & operator/=(DFrame & f, double scalar);


    /************************** Increment and decrement **********************/

    ///
    /// Increments \p f by 1.0. Returns value of \p f after incrementation.
    ///
    friend DFrame & operator++(DFrame & f);
    ///
    /// Decrements \p f by 1.0. Returns value of \p f after decrementation.
    ///
    friend DFrame & operator--(DFrame & f);
    ///
    /// Increments \p f by 1.0. Returns value of \p f before incrementation.
    ///
    friend DFrame   operator++(DFrame & f, int);
    ///
    /// Decrements \p f by 1.0. Returns value of \p f before decrementation.
    ///
    friend DFrame   operator--(DFrame & f, int);


    /************************** Floor, ceil, and round ***********************/

    ///
    /// Returns the closest exact integer frame if \p f is an integer frame;
    /// otherwise returns the largest exact integer frame not greater than \p f.
    ///
    static DFrame floor(const DFrame & f) { return DFrame(std::floor(f.value_ + EPS())); }
    ///
    /// Returns the closest exact integer frame if \p f is an integer frame;
    /// otherwise returns the smaller exact integer frame not less than \p f.
    ///
    static DFrame ceil (const DFrame & f) { return DFrame(std::ceil (f.value_ - EPS())); }
    ///
    /// Returns the closest exact integer frame of \p f.
    ///
    static DFrame round(const DFrame & f) { return DFrame(std::floor(f.value_ + 0.5)); }


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
    /// Returns DFrame::floor(*this) as an int. We remind that DFrame::floor(*this)
    /// is always an exact integer frame.
    ///
    int    toInt   () const { return DFrame::floor(*this).value_; }


    /************************** Convert from and to time in seconds **********/

    ///
    /// Returns the time in seconds corresponding to this DFrame, according to the
    /// given \p fps.
    ///
    double toSeconds(double fps) const { return value_ / fps; }
    ///
    /// Returns the DFrame corresponding to the given time in seconds \p t,
    /// according to the given \p fps.
    ///
    static DFrame fromSeconds(double t, double fps) { return DFrame(t * fps); }
    
private:
    double value_;
};

// Comparison operators
inline bool operator==(const DFrame & f1, const DFrame & f2) { return std::abs(f1.value_ - f2.value_) <= DFrame::EPS(); }
inline bool operator!=(const DFrame & f1, const DFrame & f2) { return !(f1 == f2); }
inline bool operator< (const DFrame & f1, const DFrame & f2) { return (f1 != f2) && (f1.value_ < f2.value_); }
inline bool operator> (const DFrame & f1, const DFrame & f2) { return (f1 != f2) && (f1.value_ > f2.value_); }
inline bool operator<=(const DFrame & f1, const DFrame & f2) { return (f1 == f2) || (f1.value_ < f2.value_); }
inline bool operator>=(const DFrame & f1, const DFrame & f2) { return (f1 == f2) || (f1.value_ > f2.value_); }

// Arithmetic operators
inline DFrame operator+(const DFrame & f1, const DFrame & f2) { return DFrame(f1.value_ + f2.value_); }
inline DFrame operator-(const DFrame & f1, const DFrame & f2) { return DFrame(f1.value_ - f2.value_); }
inline DFrame operator*(double scalar, const DFrame & f)     { return DFrame(scalar * f.value_); }
inline DFrame operator*(const DFrame & f, double scalar)     { return DFrame(scalar * f.value_); }
inline DFrame operator/(const DFrame & f, double scalar)     { return DFrame(f.value_ / scalar); }

// Compound assignment operators
inline DFrame & operator+=(DFrame & f1, const DFrame & f2) { f1.value_ += f2.value_; return f1; }
inline DFrame & operator-=(DFrame & f1, const DFrame & f2) { f1.value_ -= f2.value_; return f1; }
inline DFrame & operator*=(DFrame & f, double scalar)     { f.value_  *= scalar;    return f;  }
inline DFrame & operator/=(DFrame & f, double scalar)     { f.value_  /= scalar;    return f;  }

// Increment and decrement
inline DFrame & operator++(DFrame & f) { f.value_ += 1.0; return f; }
inline DFrame & operator--(DFrame & f) { f.value_ -= 1.0; return f; }
inline DFrame   operator++(DFrame & f, int) { DFrame res = f; f.value_ += 1.0; return res; }
inline DFrame   operator--(DFrame & f, int) { DFrame res = f; f.value_ -= 1.0; return res; }

} // end namespace OpenVac

#endif // OPENVAC_DFRAME_H

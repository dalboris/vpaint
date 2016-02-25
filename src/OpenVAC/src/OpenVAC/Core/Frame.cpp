// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Core/Frame.h"

namespace OpenVAC
{

const double Frame::EPS = 1.0e-10;

///////////////////////////////////////////////////////////////////////////////
///
/// \fn Frame::Frame()
///
/// Constructs a zero-initialized Frame.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn Frame::Frame(double value)
///
/// Constructs a Frame initialized by the given \p value.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn friend bool Frame::operator==(const Frame & f1, const Frame & f2)
///
/// Returns \c true if \p f1 and \p f2 are epsilon-close; otherwise returns \c false.
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
///////////////////////////////////////////////////////////////////////////////
///
/// \fn friend bool Frame::operator!=(const Frame & f1, const Frame & f2)
///
/// Returns \c true if \p f1 and \p f2 are distant by more than epsilon;
/// otherwise returns \c false.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn friend bool Frame::operator< (const Frame & f1, const Frame & f2)
///
/// Returns \c true if \p f1 and \p f2 are not epsilon-close and \p f1 is
/// lesser than \p f2; otherwise returns \c false.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn friend bool Frame::operator> (const Frame & f1, const Frame & f2)
///
/// Returns \c true if \p f1 and \p f2 are not epsilon-close and \p f1 is
/// greater than \p f2; otherwise returns \c false.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn friend bool Frame::operator<=(const Frame & f1, const Frame & f2)
///
/// Returns \c true if \p f1 and \p f2 are epsilon-close or \p f1 is
/// lesser than \p f2; otherwise returns \c false.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn friend bool Frame::operator>=(const Frame & f1, const Frame & f2)
///
/// Returns \c true if \p f1 and \p f2 are epsilon-close or \p f1 is
/// greater than \p f2; otherwise returns \c false.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn friend Frame Frame::operator+(const Frame & f1, const Frame & f2)
///
/// Returns the sum of \p f1 and \p f2.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn friend Frame Frame::operator-(const Frame & f1, const Frame & f2)
///
/// Returns the difference between \p f1 and \p f2.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn friend Frame Frame::operator*(double scalar, const Frame & f)
///
/// Returns the multiplication of \p f by \p scalar.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn friend Frame Frame::operator*(const Frame & f, double scalar)
///
/// Returns the multiplication of \p f by \p scalar.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn friend Frame Frame::operator/(const Frame & f, double scalar)
///
/// Returns the division of \p f by \p scalar.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn friend Frame & Frame::operator+=(Frame & f1, const Frame & f2)
///
/// Adds \p f2 to \p f1.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn friend Frame & Frame::operator-=(Frame & f1, const Frame & f2)
///
/// Substracts \p f2 from \p f1.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn friend Frame & Frame::operator*=(Frame & f, double scalar)
///
/// Multiplies \p f by \p scalar.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn friend Frame & Frame::operator/=(Frame & f, double scalar)
///
/// Divides \p f by \p scalar.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn friend Frame & Frame::operator++(Frame & f)
///
/// Increments \p f by 1.0. Returns value of \p f after incrementation.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn friend Frame & Frame::operator--(Frame & f)
///
/// Decrements \p f by 1.0. Returns value of \p f after decrementation.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn friend Frame   Frame::operator++(Frame & f, int)
///
/// Increments \p f by 1.0. Returns value of \p f before incrementation.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn friend Frame   Frame::operator--(Frame & f, int)
///
/// Decrements \p f by 1.0. Returns value of \p f before decrementation.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn Frame Frame::floor(const Frame & f)
///
/// Returns the closest exact integer frame if \p f is an integer frame;
/// otherwise returns the largest exact integer frame not greater than \p f.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn static Frame Frame::ceil (const Frame & f)
///
/// Returns the closest exact integer frame if \p f is an integer frame;
/// otherwise returns the smaller exact integer frame not less than \p f.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn static Frame Frame::round(const Frame & f)
///
/// Returns the closest exact integer frame of \p f.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn bool Frame::isInteger()
///
/// Returns \c true if \p f is epsilon-close to an exact integer frame;
/// otherwise returns false.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn bool Frame::isSubframe()
///
/// Returns \c false if \p f is epsilon-close to an exact integer frame;
/// otherwise returns true.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn double Frame::toDouble()
///
/// Returns the underlying double representing the frame.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn int Frame::toInt()
///
/// Returns Frame::floor(*this) as an int. We remind that Frame::floor(*this)
/// is always an exact integer frame.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn double Frame::toSeconds(double fps) const
///
/// Returns the time in seconds corresponding to this Frame, according to the
/// given \p fps.
///
///////////////////////////////////////////////////////////////////////////////
///
/// \fn static Frame Frame::fromSeconds(double t, double fps)
///
/// Returns the Frame corresponding to the given time in seconds \p t,
/// according to the given \p fps.
///
///////////////////////////////////////////////////////////////////////////////

}

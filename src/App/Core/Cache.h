// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef CACHE_H
#define CACHE_H

/// \class Cache
/// \brief A class template to store cached values
///
/// Usage:
/// \code
/// // class whose value b() is an expensive function of a(), and therefore
/// // we want to cache it.
/// class Foo
/// {
///     int a_;
///     mutable Cache<int> b_;
///
/// public:
///     void setA(int a)
///     {
///         a_ = a;
///         b_.setDirty();
///     }
///
///     int a() const
///     {
///         return a_;
///     }
///
///     int b() const
///     {
///         if (b_.isDirty())
///         {
///             b_.setValue(computeBFromA(a_));
///         {
///         return b_.value();
///     }
/// };
/// \endcode
///
template <class T>
class Cache
{
public:
    /// Returns whether the cache is up-to-date or not.
    ///
    bool isDirty() const
    {
        return isDirty_;
    }

    /// Returns the value currently stored. Might not be up-to-date.
    /// It is your responsibility to check isDirty() first, and
    /// update the value if required.
    ///
    const T & value() const
    {
        return value_;
    }

    /// Specifies that the cached value is not anymore up-to-date (for
    /// instance, because one external value it depends on was changed).
    ///
    void setDirty()
    {
        isDirty_ = true;
    }

    /// Sets a new value for the cached. Also specifies that the cache is
    /// now up-to-date.
    ///
    void setValue(const T & value)
    {
        value_ = value;
        isDirty_ = false;
    }

private:
    bool isDirty_ = true;
    T value_;
};

#endif // CACHE_H

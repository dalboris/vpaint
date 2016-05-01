// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef MEMORY_H
#define MEMORY_H

#include <OpenVac/Core/Memory.h>

/****************************** UniquePtr ************************************/

template <class T>
using UniquePtr = OpenVac::UniquePtr<T>;


/****************************** SharedPtr ************************************/

template <class T>
using SharedPtr = OpenVac::SharedPtr<T>;


/******************************* WeakPtr *************************************/

template <class T>
using WeakPtr = OpenVac::WeakPtr<T>;

#endif // MEMORY_H

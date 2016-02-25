// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef IDMANAGER_H
#define IDMANAGER_H

#include <map>

namespace OpenVAC
{

/// \class IdManager Core/IdManager.h
/// \brief The IdManager class to assign unique IDs to objects, and retrieve objects by their IDs.

template <class Id, class T>
class IdManager
{
public:


private:
    std::map<Id, T> map_;
};

}

#endif // IDMANAGER_H

// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Operator.h"

namespace OpenVAC
{

Operator::Operator(VAC * vac) :
    vac_(vac),
    isValidated_(false),
    isComputed_(false)
{
}

VAC * Operator::vac() const
{
    return vac_;
}

bool Operator::isValid()
{
    if (!isValidated_)
    {
        valid_ = isValid_();
        isValidated_ = true;
    }
    return valid_;
}

bool Operator::apply()
{
    if (isValid())
    {
        if (!isComputed_)
        {
            compute_();
            isComputed_ = true;
        }
        apply_();
        return true;
    }
    else
    {
        return false;
    }
}

void Operator::apply_()
{
    // XXX TODO
}

}

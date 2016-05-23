// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "VecCurve.h"

VecCurve::VecCurve()
{

}

void VecCurve::clear()
{
    samples_.clear();
}

void VecCurve::addSample(const glm::vec2 & position, float width)
{
    // XXX TODO
}

int VecCurve::numSamples() const
{
    return samples_.size();
}

const VecCurveSample & VecCurve::sample(int i) const
{
    return samples_.at(i);
}

VecCurveSample VecCurve::sample(double s) const
{
    // XXX TODO
}

double VecCurve::length() const
{
    if (size() == 0)
    {
        return 0.0;
    }
    else
    {
        return samples_.back().arclength;
    }
}

size_t VecCurve::size() const
{
    return samples_.size();
}

void VecCurve::push_back(const VecCurveSample & sample)
{
    samples_.push_back(sample);
}

void VecCurve::push_back(VecCurveSample && sample)
{
    samples_.push_back(std::move(sample));
}

VecCurveSample & VecCurve::operator[](int i)
{
    return samples_[i];
}

std::vector<VecCurveSample> & VecCurve::samples()
{
    return samples_;
}

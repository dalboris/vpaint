// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Curve.h"

Curve::Curve()
{

}

void Curve::clear()
{
    samples_.clear();
}

void Curve::addSample(const glm::vec2 & position, float width)
{
    // XXX TODO
}

int Curve::numSamples() const
{
    return samples_.size();
}

const CurveSample & Curve::sample(int i) const
{
    return samples_.at(i);
}

CurveSample Curve::sample(double s) const
{
    // XXX TODO
}

double Curve::length() const
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

size_t Curve::size() const
{
    return samples_.size();
}

void Curve::push_back(const CurveSample & sample)
{
    samples_.push_back(sample);
}

void Curve::push_back(CurveSample && sample)
{
    samples_.push_back(std::move(sample));
}

CurveSample & Curve::operator[](int i)
{
    return samples_[i];
}

std::vector<CurveSample> & Curve::samples()
{
    return samples_;
}

// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VGEOMETRY_VCURVE_H
#define VGEOMETRY_VCURVE_H

#include "VCurveSample.h"
#include "VCurveInputSample.h"
#include "QuadraticCurve.h"

#include <vector>

namespace VGeometry
{

/// \class VCurve
/// \brief A class to represent a smooth curve with variable thickness.
///
class VCurve
{
public:
    /// Constructs an empty curve.
    ///
    VCurve();

    /// Clears the curve.
    ///
    void clear();

    /// Initiates interactive fitting from input samples. You need to first
    /// call beginFit() once, then call continueFit() once per sample to fit, then
    /// call endFit() after the last sample. The curve is valid and can be
    /// rendered at any moment during the fitting.
    ///
    /// Note that since the fitting scheme used by VCurce is local, only the last few
    /// curve samples are affected by any given new input sample to fit. Therefore,
    /// only minimal computation has to performed. Each continueFit() has constant-time
    /// complexity on average.
    ///
    void beginFit();

    /// Continues interactive fitting from input samples.
    ///
    void continueFit(const VCurveInputSample & inputSample);

    /// Ends interactive fitting from input samples.
    ///
    void endFit();

    /// Returns the number of samples in this curve.
    ///
    size_t numSamples() const;

    /// Returns the i-th sample.
    ///
    const VCurveSample & sample(unsigned int i) const;

    /// Returns the vector of samples as a modifiable reference.
    /// Only use this if you know what you are doing.
    ///
    std::vector<VCurveSample> & samples();

    /// Returns the length of the curve.
    ///
    double length() const;

private:
    // Append input samples, possibly discarding samples if too close from
    // one another.
    //
    void appendInputSample_(const VCurveInputSample & inputSample);
    std::vector<VCurveInputSample> inputSamples_;

    // Regularized positions and widths.
    //
    // regCubicFits_ and regWidths_ have the same size as inputSamples_, but
    // with better spacing and minimal smoothing to fix artefacts caused by
    // sampling errors (e.g., samples perfectly aligned on a grid due to pixel
    // precision).
    //
    // regCubicFits_ are local cubic fits used to compute regPositions_.
    //
    void computeRegPositions_();
    void computeRegWidths_();
    void computeRegFits_();
    void averageRegFits_();
    std::vector<QuadraticCurve> regFits_;
    std::vector<glm::dvec2> regPositions_;
    std::vector<double> regWidths_;

    // Final curve samples
    //
    void computeFinalSamples_();
    std::vector<VCurveSample> samples_;
};

} // end namespace VGeometry

#endif // VGEOMETRY_VCURVE_H

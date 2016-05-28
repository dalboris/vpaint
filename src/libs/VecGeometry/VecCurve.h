// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef CURVE_H
#define CURVE_H

#include "VecCurveSample.h"
#include "VecCurveInputSample.h"
#include "VecFitter.h"

#include <vector>

/// \class VecCurve
/// \brief A class to represent a piecewise linear curve.
///
/// There are two ways to use this class:
///
///    1. You let it compute tangents, normals, arclengths, and angles for you.
///       In this case, only use the following methods to construct the curve:
///
///         - Curve()
///         - void clear()
///         - void addSample(const glm::vec2 & position, double width)
///
///       And the following methods to access the curve:
///
///         - int numSamples() const
///         - CurveSample sample(int i) const
///         - CurveSample sample(double s) const
///         - double length() const
///
///    2. You know what you are doing and want to modify the data directly,
///       in which case you can also use (i.e., you use this class merely
///       as a std::vector<VecCurveSample>):
///
///         - size_t size() const
///         - void push_back(const CurveSample & sample)
///         - void push_back(CurveSample && sample)
///         - CurveSample & operator[](int i)
///         - std::vector<CurveSample> & samples()
///
class VecCurve
{
public:
    /// Constructs an empty curve.
    ///
    VecCurve();

    /// Clears the curve.
    ///
    void clear();

    /// Adds a sample to the curve, automatically computing tangents, normals,
    /// arclengths, and angles. This typically also modifies the previous
    /// sample, since its tangent is affected by the new sample.
    ///
    void addSample(const VecCurveInputSample & inputSample);

    /// Returns the number of samples in this curve.
    ///
    size_t numSamples() const;

    /// Returns the i-th sample.
    ///
    const VecCurveSample & sample(unsigned int i) const;

    /// Returns the sample at the given arclength \p s, interpolating
    /// neighbouring samples.
    ///
    /// XXX Not implememted yet: returns an uninitialized sample.
    ///
    VecCurveSample sample(double s) const;

    /// Returns the length of the curve.
    ///
    double length() const;

    /// Return the number of samples in this curve. This is the same as
    /// numSamples(), provided for convenience for familiarity with the STL.
    ///
    size_t size() const;

    /// Appends a sample to the vector of samples. Passed by const reference.
    ///
    void push_back(const VecCurveSample & sample);

    /// Appends a sample to the vector of samples. Passed by rvalue.
    ///
    void push_back(VecCurveSample && sample);

    /// Returns the i-th sample as a modifiable reference.
    ///
    VecCurveSample & operator[](int i);

    /// Returns the vector of samples as a modifiable reference.
    ///
    std::vector<VecCurveSample> & samples();

//private:
    // ---- Smoothing algorithm data ----

    VecCurveSample toCurveSample_(const VecCurveInputSample & inputSample);

    // Raw input with samples within resoltion removed
    bool addSampleIfNotTooCloseFromPrevious_(const VecCurveInputSample & inputSample);
    std::vector<VecCurveInputSample> inputSamples_;
    VecCurveInputSample lastSample_;

    // Uniform sampling of input
    void computeInputUniformSampling_();
    float samplingRate_;
    std::vector<glm::vec2> inputUniformSamplingPosition_;
    std::vector<float> inputUniformSamplingWidth_;

    // non-normalized bell-shaped function, centered at 0.5:
    //   at u=0   : w=0 and w'=0
    //   at u=0.5 : w>0 and w'=0
    //   at u=1   : w=0 and w'=0
    template <typename T>
    inline T w_(T u) { return u*u*(1-u)*(1-u); }

    // Smoothed uniform sampling
    // Old
    void smoothUniformSampling_();
    size_t numFitSamples_;
    std::vector<VecFitter> fitters_;
    std::vector<glm::vec2> smoothedUniformSamplingPosition_;
    std::vector<float> smoothedUniformSamplingWidth_;
    // New
    void computeNoiseFitters_();
    void averageNoiseFitters_();
    std::vector<VecFitter> noiseFitters_;
    std::vector<glm::vec2> regularizedPositions_;
    std::vector<double> regularizedWidth_;
    // For comparison
    void computeConvolution_();
    std::vector<glm::vec2> convolutedSamples_;

    // Final samples to render
    void computeFinalSamples_();
    std::vector<VecCurveSample> samples_;

};

#endif // CURVE_H

// Copyright (C) 2012-2019 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SCULPT_CURVE_H
#define SCULPT_CURVE_H

/*
 * Class holding a curve providing:
 *   - fitting         provides a neatened curve from mouse input
 *   - oversketching   modify the curve by drawing a rectified sub-curve
 *   - sculpting       modify the curve by smoothing and dragging vertices
 *   - splitting       the curve in different sub-curves)
 *   - intersection    compute self-intersections and intersections with other curves
 *
 * The template parameter T represents a vertex of the curve. It must:
 *  - have the public members `x` and `y`
 *  - have the public members `width()`
 *  - have a default constructor providing appropriate default values (must be zero for x and y)
 *  - have a public method T lerp(double u, const T & other) const;
 *  - have addition, substraction and scalar product
 *  - have distanceTo
 *
 * Sampling:
 *  users can set a sampling size ds. This means that the distance between two vertices will be
 *  at *most* ds, but it can be less. For instance, the method resample(ds) uses the values
 *  curve(i*ds) with curve(s) being linear by part. This can "cut corners".
 *
 *  Invariant: after calling resample(), the distance between two consecutive sample is
 *                0 < epsilon() < d(pi,pi+1) < ds()
 *
 * Note that for loops (= closed edges), the start/end point is duplicated,
 * that is, the first and last samples are equal. This makes it easier for code
 * who doesn't care about closedness, for example computing arclength
 * distances, closest point, etc. This means that if you specify the vertices
 * explicitly via setVertices, you are expected to provide this start/end point
 * twice.
 *
 */

// if debug
#include <iostream>

#include <vector>
#include <list>
#include <queue>
#include <cmath>
#include <algorithm>
#include <cassert>

#include <Eigen/Core>
#include <Eigen/LU>
#include <Eigen/StdVector>

#ifndef DEFINE_STD_VECTOR_INSERTION_OPERATOR
#define DEFINE_STD_VECTOR_INSERTION_OPERATOR
// Defines insertion operator (<<) for std::vector, for convenience
template<class T> std::vector<T> & operator<<(std::vector<T> & v, const T & x)
{ v.push_back(x); return v; }
template<class T> std::vector<T,Eigen::aligned_allocator<T> > & operator<<(std::vector<T,Eigen::aligned_allocator<T> > & v, const T & x)
{ v.push_back(x); return v; }
#endif

namespace SculptCurve
{

inline bool isnan(double x)
{
#ifdef _WIN32
    return _isnan(x);
#else
    return std::isnan(x);
#endif
}

struct Intersection
{
    double s, t;
    Intersection(double s, double t): s(s), t(t) {}

    bool operator< (const Intersection & other) const
    {
        return s < other.s;
    }
};


template<class T>
class Curve
{
public:
    // -------- Construction and Destruction --------

    // Construct an empty curve. Optionally, specify a sampling rate
    Curve(double ds = 5.0) :
        dirtyArclengths_(false), isClosed_(false), sketchInProgress_(false),
        N_(10), fitterType_(QUARTIC_BEZIER_FITTER),
        ds_(ds), lastDs_(-1) {}

    // Construct a straight line
    Curve(const T & start, const T & end, double ds = 5.0) :
        dirtyArclengths_(true), isClosed_(false), sketchInProgress_(false),
        N_(20), fitterType_(QUARTIC_BEZIER_FITTER),
        ds_(ds), lastDs_(-1)
    {
        vertices_.push_back(start);
        vertices_.push_back(end);
        resample(true);
    }

    // Reinitialize curve
    void clear() {
        vertices_.clear(); arclengths_.clear(); lastDs_ = -1; dirtyArclengths_ = false; isClosed_ = false;


        p_.clear(); // raw input from mouse
        qTemp_.clear(); // temp vertices
        clearFits_();
    }

    // must ensure that the first vertex is equal to the last
    // by default, sculpt curve never create a loop, it must be explicitly
    // created using the method below, so that it will behave as a loop
    void makeLoop()
    {
        isClosed_ = true;
    }

    double epsilon() const
    {
        return 1e-6;
    }

    // -------- Sketching and Fitting --------

    void beginSketch(double x, double y) { T v; v.setX(x); v.setY(y); beginSketch(v); }
    void beginSketch(const T & vertex)
    {

        // Initialization
        clear();

        // add first point
        p_.push_back(Input(vertex,0));
        pushFirstVertex_(vertex);

        lastFinalS_ = 0;
        sketchInProgress_ = true;
    }

    void continueSketch(double x, double y) { T v; v.setX(x); v.setY(y); continueSketch(v); }
    void continueSketch(const T & vertex)
    {

        if(sketchInProgress_)
        {


        // add input point
        //Eigen::Vector2d p(x,y);
            double dx = vertex.x()-p_.back().p.x();
            double dy = vertex.y()-p_.back().p.y();
        double distSquared = dx*dx + dy*dy;
        if(!(distSquared>0))
            return;
        p_.push_back(Input( vertex, p_.back().s + std::sqrt(distSquared)));

        // erase previous temporary data
        qTemp_.clear();

        // compute new fit
        if(p_.size() < (unsigned int) N_)
        {
            // Compute fit
            Fitter * fit = fitter(fitterType_, p_, 0, p_.size(), ds_);

            T q = vertices_.back(); // = q_[0]
            double s = lastFinalS_;                 // = 0

            // sample from fit
            // while we have not reached the end
            while(p_.back().s - s > ds_)
            {
                // add a new vertex
                s += 0.75*ds_;
                q = phi_(s, fit);
                qTemp_.push_back(q);
            }
            // add last vertex
            T lastP = p_.back().p;
            qTemp_.push_back(lastP);

            delete fit;
        }
        else
        {
            // compute new fitting
            Fitter * fit = fitter(fitterType_,p_,p_.size()-N_,N_,ds_);
            fits_ << fit;

            T q = vertices_.back();
            //double qt = qt_.last();
            double s = lastFinalS_;

            // while we have not reached the end
            while(p_.back().s - s > ds_)
            {
                // add a new vertex
                s += 0.75*ds_;
                q = phi_(s);
                if(s<=p_[p_.size()-N_+1].s)
                {
                    pushVertex_(q);
                    lastFinalS_ = s;
                }
                else
                {
                    qTemp_.push_back(q);
                }
                //viz_.addVertex(q[0],q[1]);
            }

            // add last vertex
            T lastP = p_.back().p;
            qTemp_.push_back(lastP);
        }

        lastDs_ = -1;
        }
    }

    void endSketch()
    {
        for(T vertex : qTemp_)
            pushVertex_(vertex);
        qTemp_.clear();

        //vertices_.insert(vertices_.end(), qTemp_.begin(), qTemp_.end());
        clearFits_();
        //makeQuasiUniform_();
        //computeQS_();

        p_.clear(); // clear raw input (used as a test to know is curve is being drawn)




        sketchInProgress_ = false;
        resample(true);

    }

    // -------- Sampling --------

    int size() const
    {
        return vertices_.size() + qTemp_.size();
    }
    T operator[] (int i) const
    {
        int k = i-vertices_.size();
        if(k<0)
            return vertices_[i];
        else
            return qTemp_[k];
    }
    double arclength(int i) const
    {
        precomputeArclengths_();
        return arclengths_[i];
    }

    T start() const
    {
        if(size())
            return operator[](0);
        else
            return T();
    }
    T end() const
    {
        if(size())
            return operator[](size()-1);
        else
            return T();
    }



    double ds() const { return ds_; }
    void setDs(double ds) {  ds_ = ds; }
    void resample(double ds)    { setDs(ds); resample(); }
    void resample(bool force = false)
    {
        // prevent resampling a curve already sampled at the same rate
        if(!force)
        {
            if(lastDs_ == ds_)
                return;
            else
                lastDs_ = ds_;
        }

        // We'll work on a linked list for fast insertion/deletion in the middle
        typedef std::list<T,Eigen::aligned_allocator<T> > SampleList;
        SampleList samples;

        // First pass: copy all non-NaN samples to the list
        double defaultWidth = 10;
        int n = 0;
        for(int i=0; i<size(); ++i)
        {
            T sample = operator[](i);
            if(isnan(sample.width()))
            {
                sample.setWidth(defaultWidth);
            }

            if(!isnan(sample.x()) && !isnan(sample.y()))
            {
                samples.push_back(sample);
                ++n;
            }
        }

        // Step 1: While(n>4), Remove all (d < ds/2). Remove prelast if (dlast < ds/4)
        //         While(n<=4), Remove all (d < eps/2), and push (eps/2 <= d < eps) to (d == eps). Remove prelast if (dlast < eps/2)
        double halfDs = 0.5 * ds();
        double quarterDs = 0.25 * ds();
        double eps = epsilon();
        double halfEps = 0.5*eps;
        if(n >= 3)
        {
            typename SampleList::iterator it1 = samples.begin();
            typename SampleList::iterator it2 = it1; ++it2;
            typename SampleList::iterator itLast = samples.end(); --itLast;
            // Here is the current state of the linked list:
            //  begin=it1  it2                          last       end
            //        ()-->()-->()-->()-->()-->()-->()-->()------>(NULL)
            while(it2 != itLast)
            {
                double d = it1->distanceTo(*it2); // note: could use squared distance, would be more efficient
                if(n<=4)
                {
                    if(d<halfEps)
                    {
                        it2 = samples.erase(it2);
                        --n;
                    }
                    else
                    {
                        if(d<eps)
                        {
                            *it2 = it1->lerp(eps/d,*it2);
                        }
                        ++it1;
                        ++it2;
                    }
                }
                else
                {
                    if(d<halfDs)
                    {
                        it2 = samples.erase(it2);
                        --n;
                    }
                    else
                    {
                        ++it1;
                        ++it2;
                    }
                }
            }
            // Here is the current state of the linked list:
            //       begin                     it1       it2=last  end
            //        ()------->()------->()-->()------->()------>(NULL)
            //
            if(it1 != samples.begin())
            {
                double d = it1->distanceTo(*it2); // note: could use squared distance, would be more efficient
                if(n<=4)
                {
                    if(d<halfEps)
                    {
                        samples.erase(it1);
                        --n;
                    }
                }
                else
                {
                    if(d<quarterDs)
                    {
                        samples.erase(it1);
                        --n;
                    }
                }
            }
        }

        // Step 2: Handle trivial cases
        bool subdivide = false;
        if(n<2)
        {
            if(n == 0)
            {
                T sample;
                sample.setWidth(defaultWidth);
                samples.push_back(sample);
                ++n;
            }
            // now, n == 1
            typename SampleList::iterator it = samples.begin();
            T sample0 = *it;
            T sample1 = sample0; sample1.setX(sample0.x()+eps);
            T sample2 = sample1; sample2.setY(sample1.y()+eps);
            T sample3 = sample0;
            samples.push_back(sample1);
            ++n;
            samples.push_back(sample2);
            ++n;
            samples.push_back(sample3);
            ++n;
        }
        else if(n == 2)
        {
            typename SampleList::iterator it1 = samples.begin();
            typename SampleList::iterator it2 = it1; ++it2;
            double d = it1->distanceTo(*it2); // note: could use squared distance, would be more efficient
            if(d<halfEps)
            {
                // same as n<2, we have guarantee that the last distance will be > halfEps
                T sample0 = *it1;
                T sample1 = sample0; sample1.setX(sample0.x()+eps);
                T sample2 = sample1; sample2.setY(sample1.y()+eps);
                samples.insert(it2,sample1); // insert just before it2
                ++n;
                samples.insert(it2,sample2); // insert just before it2
                ++n;
            }
            else if(d<eps)
            {
                // we have safe access to tangent
                T sample0 = *it1;
                T sample1 = sample0; // copy width, pos will be overriden
                T sample2 = sample0;// copy width, pos will be overriden
                T sample3 = *it2;
                Eigen::Vector2d p0(sample0.x(),sample0.y());
                Eigen::Vector2d p3(sample3.x(),sample3.y());
                Eigen::Vector2d u = p3 - p0;
                u.normalize(); // fine because d > eps/2
                Eigen::Vector2d v(-u[1],u[0]); // normal
                double x = std::sqrt(eps*eps - 0.25*(eps-d)*(eps-d)); // cf. page 575 of my notes.
                Eigen::Vector2d p1 = 0.5*(p0+p3) + x*v - halfEps*u;
                Eigen::Vector2d p2 = 0.5*(p0+p3) + x*v + halfEps*u;
                sample1.setX(p1[0]);
                sample1.setY(p1[1]);
                sample2.setX(p2[0]);
                sample2.setY(p2[1]);
                samples.insert(it2,sample1); // insert just before it2
                ++n;
                samples.insert(it2,sample2); // insert just before it2
                ++n;
            }
            else
            {
                // subdivide explicitely
                n = std::floor(d/ds()) + 2;
                if(n>2)
                {
                    // we have safe access to tangent
                    for(int i=1; i<n-1; ++i)
                    {
                        double u = (double) i / (double) (n-1);
                        T sample = it1->lerp(u,*it2);
                        samples.insert(it2,sample); // insert just before it2
                    }
                }
            }
        }
        else if(n == 3)
        {
            typename SampleList::iterator it1 = samples.begin();
            typename SampleList::iterator it2 = it1; ++it2;
            typename SampleList::iterator it3 = it2; ++it3;
            // Since p1 has survived the cleaning, we know ||p1-p0|| > eps
            double d = it1->distanceTo(*it3); // note: could use squared distance, would be more efficient
            if(d<halfEps)
            {
                T sample0 = *it1;
                T sample1 = *it2;
                T sample2 = sample0;// copy width, pos will be overriden
                //T sample3 = *it3;
                Eigen::Vector2d p0(sample0.x(),sample0.y());
                Eigen::Vector2d p1(sample1.x(),sample1.y());
                Eigen::Vector2d u = p1 - p0;
                u.normalize(); // fine because we know ||p1-p0|| > eps
                Eigen::Vector2d v(-u[1],u[0]); // normal
                Eigen::Vector2d p2 = p1 - eps*v;
                sample2.setX(p2[0]);
                sample2.setY(p2[1]);
                samples.insert(it3,sample2); // insert just before it3
                ++n;
            }
            else
            {
                // we're good
            }
            // In any case, since ||p1-p0|| or ||p2-p1|| can be arbitrarily large
            subdivide = true;
        }
        else
        {
            subdivide = true;
        }

        // Step 3: Subdivision scheme
        if(subdivide) // Note: this implies n>=3
        {
            SampleList subdividedSamples;
            bool subdivideAgain = true;
            while(subdivideAgain)
            {
                // Initialization
                subdivideAgain = false;
                typename SampleList::iterator it1 = samples.begin();
                typename SampleList::iterator it2 = it1; ++it2;
                typename SampleList::iterator itEnd = samples.end();
                typename SampleList::iterator itFirst = it1;
                typename SampleList::iterator itLast = itEnd; --itLast;
                subdividedSamples.push_back(*it1);

                // Main loop through list
                while(it2 != itEnd)
                {
                    double d = it1->distanceTo(*it2); // note: could use squared distance, would be more efficient
                    if(d>ds()) // should subdivide
                    {
                        // compute new sample using 4-point subdivision scheme [Dyn 1987]
                        typename SampleList::iterator it0 = it1;
                        if (isClosed_ && it0 == itFirst) it0 = itLast;
                        if(it0 != itFirst) --it0;
                        typename SampleList::iterator it3 = it2;
                        if (isClosed_ && it3 == itLast) it3 = itFirst;
                        if(it3 != itLast) ++it3;
                        T sample0 = *it0;
                        T sample1 = *it1;
                        T sample2 = *it2;
                        T sample3 = *it3;
                        //Eigen::Vector2d p0(sample0.x(),sample0.y());
                        //Eigen::Vector2d p1(sample1.x(),sample1.y());
                        //Eigen::Vector2d p2(sample2.x(),sample2.y());
                        //Eigen::Vector2d p3(sample3.x(),sample3.y());
                        double w = 0.0625; // i.e., 1/16
                        double halfPlusW = 0.5625; // i.e., 1/2 + 1/16
                        //Eigen::Vector2d p = halfPlusW*(p1+p2) + w*(p0+p3);
                        T newSample = (sample1+sample2)*halfPlusW - (sample0+sample3)*w;

                        // insert
                        subdividedSamples.push_back(newSample);

                        // recurse
                        subdivideAgain = true;
                    }

                    // insert it2 anyway
                    subdividedSamples.push_back(*it2);

                    // advance in list
                    ++it1;
                    ++it2;
                }

                // What to do at the end
                samples.swap(subdividedSamples);
                subdividedSamples.clear();
            }
        }

        // Copy back the list to the vector
        vertices_.clear();
        typename SampleList::iterator itBegin = samples.begin();
        typename SampleList::iterator itEnd = samples.end();
        for(typename SampleList::iterator it = itBegin; it != itEnd; ++it)
            vertices_.push_back(*it);
        setDirtyArclengths_();
    }

    // directly set the curve to be the provided vertices, for instance
    // coming from another neatening algorithm or curve representation
    // keep the loopness it has before calling the function
    void setVertices(const std::vector<T,Eigen::aligned_allocator<T> > & newVertices)
    {
        // clear but keep loopness
        bool loopTmp = isClosed_;
        clear();
        isClosed_ = loopTmp;

        // set vertices
        vertices_ = newVertices;
        setDirtyArclengths_();
    }

    // -------- Continuous curve --------

    // Note: these functions ignore whatever is in qTemp
    //       so might be unexpected behaviour if called while drawing

    double length() const
    {
        precomputeArclengths_();
        return arclengths_.back();
    }

    T operator() (double s) const
    {
        int n = vertices_.size();
        assert(n>0);
        if(n == 1)
            return vertices_.front();
        else
            return interpolatedVertex_(s);
    }

    // -------- Apply affine transform --------


    Curve<T> transformed(const Eigen::Affine2d & xf)
    {
        Curve<T> res(*this);
        res.transform(xf);
        return res;
    }

    void transform(const Eigen::Affine2d & xf)
    {
        for (unsigned int i=0; i<vertices_.size(); ++i)
        {
            Eigen::Vector2d p(vertices_[i].x(), vertices_[i].y());
            p = xf*p;
            vertices_[i].setX(p[0]);
            vertices_[i].setY(p[1]);
        }

        resample(true);
    }

    // -------- Sculpting --------

    void translate(double dx, double dy)
    {
        // very fast, no need to recompute arclength :-)

        int n = vertices_.size();
        for(int i = 0; i<n; ++i)
        {
            vertices_[i].setX(vertices_[i].x() + dx);
            vertices_[i].setY(vertices_[i].y() + dy);
        }
    }

    // return -1 if no vertices
    struct ClosestVertex { int i; double d; };
    ClosestVertex findClosestVertex(double x, double y) const
    {
        double minD2 = std::numeric_limits<double>::max();
        int i = -1;
        int minI = i;
        for(auto v: vertices_)
        {
            ++i;
            double dx = x-v.x();
            double dy = y-v.y();
            double d2 = dx*dx + dy*dy;
            if(d2<minD2)
            {
                minD2 = d2;
                minI = i;
            }
        }
        ClosestVertex res = { minI, sqrt(minD2) };
        return res;
    }

    double prepareSculpt(double x, double y, double radius)
    {
        ClosestVertex v = findClosestVertex(x,y);
        sculptIndex_ = v.i;
        sculptRadius_ = radius;
        return v.d;
    }

    double arclengthOfSculptVertex() const
    {
        if(sculptIndex_>=0 && sculptIndex_<size() )
        {
            precomputeArclengths_();
            return arclengths_[sculptIndex_];
        }
        else return 0;
    }

    T sculptVertex() const { if(sculptIndex_>=0 && sculptIndex_<size() ) return vertices_[sculptIndex_]; else return T(); }
    int sculptVertexIndex() const { return sculptIndex_; }

    double w_(double s, double radius) const // s : arclength distance from sculptVertex to processed vertex
    {
        if( s>radius || s<-radius)
            return 0;

        double a = (s - radius);
        double b = (s + radius);
        double radius2 = radius * radius;
        double radius4 = radius2 * radius2;
        return a*a*b*b/radius4;
    }

    double w2_(double d, double r0, double w0) const // cf p 296
    {
        if( d>r0 || d<-r0)
            return w0;

        double a = (d - r0);
        double b = (d + r0);
        double radius2 = r0 * r0;
        double radius4 = radius2 * radius2;
        return a*a*b*b/radius4 * (1-w0) + w0;
    }
    double w_(double s) const // s : arclength distance from sculptVertex to processed vertex
    {
        return w_(s, sculptRadius_);
    }

    void beginSculptDeform(double x, double y)
    {
        // This method needs arclengths
        precomputeArclengths_();

        // Store start x and y
        sculptStartX_ = x;
        sculptStartY_ = y;

        // The following list store the vertices to deform
        sculptTemp_.clear();

        // In case nothing to sculpt
        if(sculptIndex_<=0 || sculptIndex_>=size()-1 )
            return;

        // add vertices to sculpt temp before
        if(isClosed_)
        {
            double l = length();
            double halfLength = 0.5 * l;
            bool handleLargeRadius = false;
            double r0 = 0;
            double w0 = 0;
            if(sculptRadius_ > halfLength)
            {
                handleLargeRadius = true;
                r0 = halfLength;
                w0 = w_(halfLength);
            }
            for(int i=0; i<size(); ++i)
            {
                // compute signed distance, loop-unaware.
                double d =  arclengths_[sculptIndex_] - arclengths_[i];

                // transform it into unsigned distance, loop-aware: 0 <= d <= length / 2
                if(d>halfLength)
                    d -= l;
                if(d<-halfLength)
                    d += l;
                if(d<0)
                    d *= -1;

                // insert vertex into sculptTemp_
                if(d > sculptRadius_)
                    continue;

                double w;
                if(handleLargeRadius)
                    w = w2_(d,r0,w0);
                else
                    w = w_(d);

                sculptTemp_ << SculptTemp(i, w, vertices_[i].x(), vertices_[i].y());
            }
        }
        else
        {
            // add sculpted vertex
            sculptTemp_ << SculptTemp(sculptIndex_, 1, vertices_[sculptIndex_].x(), vertices_[sculptIndex_].y());

            int i = sculptIndex_-1;
            while(i>=0)
            {
                double distanceToSculptVertex =  arclengths_[sculptIndex_] - arclengths_[i];
                if(distanceToSculptVertex > sculptRadius_)
                    break;

                double w = w_(distanceToSculptVertex);
                sculptTemp_ << SculptTemp(i, w, vertices_[i].x(), vertices_[i].y());
                --i;
            }
            // we want w(i == 0) == 0
            if(sculptTemp_.back().i == 0)
            {
                double dw = sculptTemp_.back().w;
                double oneMinusDw = 1 - dw;
                if(oneMinusDw > 0)
                {
                    for(unsigned int i = 1; i<sculptTemp_.size(); ++i)
                    {
                        sculptTemp_[i].w = (sculptTemp_[i].w - dw) / oneMinusDw;
                    }
                }
                else
                {
                    // this means we attempt to sculpt with one end with w = 1.
                    // this is not possible, as we want the ends not to move
                    sculptTemp_.clear();
                    return;
                }
            }

            // add vertices to sculpt temp after
            int j = sculptTemp_.size(); // only retarget from here, if needed
            i = sculptIndex_+1;
            while(i<size())
            {
                double distanceToSculptVertex =  arclengths_[i] - arclengths_[sculptIndex_];
                if(distanceToSculptVertex > sculptRadius_)
                    break;

                double w = w_(distanceToSculptVertex);
                sculptTemp_ << SculptTemp(i, w, vertices_[i].x(), vertices_[i].y());
                ++i;
            }
            // we want w(i == size()-1) == 0
            if(sculptTemp_.back().i == size()-1)
            {
                double dw = sculptTemp_.back().w;
                double oneMinusDw = 1 - dw;
                if(oneMinusDw > 0)
                {
                    for(unsigned int i = j; i<sculptTemp_.size(); ++i)
                    {
                        sculptTemp_[i].w = (sculptTemp_[i].w - dw) / oneMinusDw;
                    }
                }
                else
                {
                    // this means we attempt to sculpt with one end with w = 1.
                    // this is not possible, as we want the ends not to move
                    sculptTemp_.clear();
                    return;
                }
            }
        }
    }

    void continueSculptDeform(double x, double y)
    {
        setDirtyArclengths_();

        for(auto & v: sculptTemp_)
        {
            vertices_[v.i].setX(v.x  + v.w * (x - sculptStartX_));
            vertices_[v.i].setY(v.y  + v.w * (y - sculptStartY_));
        }
    }

    void endSculptDeform()
    {
        sculptTemp_.clear();
        resample(true);
    }

    // apply a smooth filter of radius sculptRadius_ and intensity intensity at sculptVertex_
    void sculptSmooth(double intensity)
    {
        precomputeArclengths_();
        std::vector<T,Eigen::aligned_allocator<T> > copyVertices = vertices_;
        if(!size())
            return;

        // to handle loops
        double l = length();
        double halfLength = 0.5 * l;
        bool handleLargeRadius = false;
        double r0 = 0;
        double w0 = 0;
        if(sculptRadius_ > halfLength)
        {
            handleLargeRadius = true;
            r0 = halfLength;
            w0 = w_(halfLength);
        }

        // compute first local intensity of extremities
        // this is only useful if isClosed == false
        double sSculpt = arclengthOfSculptVertex();

        for(int i=0; i<size(); ++i)
        {
            if(!isClosed_ && (i==0 || i==size()-1))
                continue;

            // for every affected vertex i
            double d = arclengths_[sculptIndex_] - arclengths_[i];
            if(isClosed_)
            {
                // transform it into unsigned distance, loop-aware: 0 <= d <= length / 2
                if(d>halfLength)
                    d -= l;
                if(d<-halfLength)
                    d += l;
                if(d<0)
                    d *= -1;
            }
            if(std::abs(d)<sculptRadius_)
            {
                // replace vertices_[i] by the weighted average of neighbours
                //double localRadius = sculptRadius_ * w_(d);
                double localRadius = sculptRadius_;
                double localIntensity;
                if(handleLargeRadius)
                    localIntensity = intensity * w2_(d,r0,w0);
                else
                    localIntensity = intensity * w_(d);
                T res;
                double sum = 0;
                for(int j=0; j<size(); ++j)
                {
                    double d2 = arclengths_[i] - arclengths_[j];
                    if(isClosed_)
                    {
                        // transform it into unsigned distance, loop-aware: 0 <= d <= length / 2
                        if(d2>halfLength)
                            d2 -= l;
                        if(d2<-halfLength)
                            d2 += l;
                        if(d2<0)
                            d2 *= -1;
                    }
                    if(std::abs(d2)<localRadius)
                    {
                        double w = exp( - 5 * d2*d2 / (double) (localRadius*localRadius) ); //w_(d2,localRadius);
                        res = res + copyVertices[j] * w;
                        sum += w;
                    }
                }
                if(sum>0)
                {
                    res = res * (1/sum);
                    double finalIntensity = localIntensity;
                    if(!isClosed_)
                    {
                        if(d>0) // left
                        {
                            double alpha = (sSculpt - d) / sSculpt;
                            finalIntensity = localIntensity * alpha;
                        }
                        else // right
                        {
                            double alpha = ((length() - sSculpt) - (-d)) / (length() - sSculpt);
                            finalIntensity = localIntensity * alpha;
                        }
                    }
                    vertices_[i] = copyVertices[i].lerp(finalIntensity, res);
                }
            }
        }
        resample(true);
    }



private:
    int sculptIndex_;
    double sculptRadius_;
    double sculptStartX_;
    double sculptStartY_;
    struct SculptTemp
    {
        SculptTemp(int i, double w, double x, double y) : i(i), w(w), x(x), y(y) {}
        int i; double w; double x; double y;
    };
    std::vector<SculptTemp> sculptTemp_;

public:



    //void beginSculptDeform()

    // -------- Intersections --------


    // helper method: returns det(U,V)
    inline static double det(double ux, double uy,
                             double vx, double vy)
    {
        return ux*vy - uy*vx;
    }


    // helper method: returns whether the segments [AB] and [CD] intersect, with a margin of error epsilon
    //                if yes, the intersection P is P = A + u * (B-A) = C + v * (D-C)
    inline static bool intersects(
            // input
            double ax, double ay,
            double bx, double by,
            double cx, double cy,
            double dx, double dy,
            // output
            double & u, double & v,
            // parameters
            double epsilon = 1e-10)
    {
        // fast pruning. In if size screen > 100 * ds, then in > 98% of the time, 4 assignments and <= 4 comparisons
        double minABx, maxABx;
        if(ax < bx)    { minABx = ax; maxABx = bx; } else { minABx = bx; maxABx = ax; }
        double minCDx, maxCDx;
        if(cx < dx)    { minCDx = cx; maxCDx = dx; } else { minCDx = dx; maxCDx = cx; }
        if(minABx > maxCDx || minCDx > maxABx)
            return false;
        // in > 99.98% of the time, <= 8 assignments and <= 8 comparisons
        double minABy, maxABy;
        if(ay < by)    { minABy = ay; maxABy = by; } else { minABy = by; maxABy = ay; }
        double minCDy, maxCDy;
        if(cy < dy)    { minCDy = cy; maxCDy = dy; } else { minCDy = dy; maxCDy = cy; }
        if(minABy > maxCDy || minCDy > maxABy)
            return false;

        // actual computation
        double det_ = det(bx-ax, by-ay, dx-cx, dy-cy); // det(AB,CD)
        double numU = det(dx-cx, dy-cy, ax-cx, ay-cy); // det(CD,AC)
        double numV = det(bx-ax, by-ay, ax-cx, ay-cy); // det(AB,AC)

        // if parallel
        if(std::abs(det_) < epsilon)
        {
            return false; // consider not intersecting if nearly parallel, by design.
                          // if you consider changing this, make sure to think about it
                          // full-time during at least one month. If you still think
                          // it is a good idea, then implement it. Then revert your changes
                          // when you realize how big this mistake was. Trust me, you will.
        }
        else
        {
            double invDet = 1/det_;
            u = numU * invDet;
            v = numV * invDet;
            if(u>=-epsilon && u<1+epsilon && v>=-epsilon && v<1+epsilon)
               return true;
            else
               return false;
        }
    }
    // helper method provided for convenience:
    //                returns whether the segments [AB] and [CD] intersect, with a margin of error epsilon
    //                if yes, the intersection P is P = A + u * (B-A) = C + v * (D-C)
    inline static bool intersects(
            // input
            T & a, T & b,
            T & c, T & d,
            // output
            double & u, double & v,
            // parameters
            double epsilon = 1e-10)
    {
        return intersects(a.x(), a.y(),
                          b.x(), b.y(),
                          c.x(), c.y(),
                          d.x(), d.y(),
                          u, v,
                          epsilon);
    }


    // Compute unclean intersections.
    // May have duplicates. May miss some if segments nearly parallel.
    // Includes "virtual intersections": when extending the end of the curve by tolerance would create a new intersection.
    // Return value not sorted.
    std::vector<Intersection> intersections(const SculptCurve::Curve<T> & other, double tolerance = 15.0) const
    {
        precomputeArclengths_();
        other.precomputeArclengths_();

        std::vector<Intersection> res;

        // Returns in trivial cases
        int n = size();
        int nOther = other.size();
        if(n<2 || nOther<2)
            return res;

        // store min/max
        double l = length();
        double lOther = other.length();
        double minS = l;
        double maxS = 0;
        double minT = lOther;
        double maxT = 0;

        double u, v;
        for(int i=0; i<n-1; ++i)
        {
            T va = (*this)[i];
            T vb = (*this)[i+1];
            for(int j=0; j<nOther-1; ++j)
            {
                T vc = other[j];
                T vd = other[j+1];

                bool doIntersect = intersects(va, vb, vc, vd, u, v);
                if(doIntersect)
                {
                    double s = (1-u)*arclengths_[i] + u*arclengths_[i+1];
                    double t = (1-v)*other.arclengths_[j] + v*other.arclengths_[j+1];
                    res.push_back(Intersection(s,t));

                    // update min/max
                    if(s<minS)
                        minS = s;
                    if(s>maxS)
                        maxS = s;
                    if(t<minT)
                        minT = t;
                    if(t>maxT)
                        maxT = t;
                }
            }
        }

        // Compute endpoints intersections
        if(minS > tolerance && !isClosed_) // start of this
        {
            T va = vertices_.front();
            T ve = (*this)(tolerance);
            T vb = ve.lerp(2.0, va);
            for(int j=0; j<nOther-1; ++j)
            {
                T vc = other[j];
                T vd = other[j+1];

                bool doIntersect = intersects(va, vb, vc, vd, u, v);
                if(doIntersect)
                {
                    double s = 0;
                    double t = (1-v)*other.arclengths_[j] + v*other.arclengths_[j+1];
                    res.push_back(Intersection(s,t));

                    // update min/max
                    if(s<minS)
                        minS = s;
                    if(s>maxS)
                        maxS = s;
                    if(t<minT)
                        minT = t;
                    if(t>maxT)
                        maxT = t;
                }
            }
        }
        if(maxS < l-tolerance && !isClosed_) // end of this
        {
            T va = vertices_.back();
            T ve = (*this)(l-tolerance);
            T vb = ve.lerp(2.0, va);
            for(int j=0; j<nOther-1; ++j)
            {
                T vc = other[j];
                T vd = other[j+1];

                bool doIntersect = intersects(va, vb, vc, vd, u, v);
                if(doIntersect)
                {
                    double s = l;
                    double t = (1-v)*other.arclengths_[j] + v*other.arclengths_[j+1];
                    res.push_back(Intersection(s,t));

                    // update min/max
                    if(s<minS)
                        minS = s;
                    if(s>maxS)
                        maxS = s;
                    if(t<minT)
                        minT = t;
                    if(t>maxT)
                        maxT = t;
                }
            }
        }
        if(minT > tolerance && !other.isClosed_) // start of other
        {
            T va = other.vertices_.front();
            T ve = other(tolerance);
            T vb = ve.lerp(2.0, va);
            for(int i=0; i<n-1; ++i)
            {
                T vc = vertices_[i];
                T vd = vertices_[i+1];

                bool doIntersect = intersects(va, vb, vc, vd, u, v);
                if(doIntersect)
                {
                    double t = 0;
                    double s = (1-v)*arclengths_[i] + v*arclengths_[i+1];
                    res.push_back(Intersection(s,t));

                    // update min/max
                    if(s<minS)
                        minS = s;
                    if(s>maxS)
                        maxS = s;
                    if(t<minT)
                        minT = t;
                    if(t>maxT)
                        maxT = t;
                }
            }
        }
        if(maxS < l-tolerance && !other.isClosed_) // end of this
        {
            T va = other.vertices_.back();
            T ve = other(lOther-tolerance);
            T vb = ve.lerp(2.0, va);
            for(int i=0; i<n-1; ++i)
            {
                T vc = vertices_[i];
                T vd = vertices_[i+1];

                bool doIntersect = intersects(va, vb, vc, vd, u, v);
                if(doIntersect)
                {
                    double t = lOther;
                    double s = (1-v)*arclengths_[i] + v*arclengths_[i+1];
                    res.push_back(Intersection(s,t));

                    // update min/max
                    if(s<minS)
                        minS = s;
                    if(s>maxS)
                        maxS = s;
                    if(t<minT)
                        minT = t;
                    if(t>maxT)
                        maxT = t;
                }
            }
        }

        return res;
    }

    // Compute unclean self-intersections.
    // May have duplicates. May miss some if segments nearly parallel.
    // Includes "virtual intersections": when extending the end of the curve by tolerance would create a new intersection.
    // Return value not sorted.
    std::vector<Intersection> selfIntersections(double tolerance = 15.0) const
    {
        precomputeArclengths_();

        std::vector<Intersection> res;

        // Returns in trivial cases
        int n = size();
        if(n<4)
            return res;

        // store min/max
        double l = length();
        double minS = l;
        double maxS = 0;

        double u, v;
        for(int i=0; i<n-3; ++i)
        {
            T va = (*this)[i];
            T vb = (*this)[i+1];
            for(int j=i+2; j<n-1; ++j)
            {
                T vc = (*this)[j];
                T vd = (*this)[j+1];

                bool doIntersect = intersects(va, vb, vc, vd, u, v);
                if(doIntersect)
                {
                    double s = (1-u)*arclengths_[i] + u*arclengths_[i+1];
                    double t = (1-v)*arclengths_[j] + v*arclengths_[j+1];
                    res.push_back(Intersection(s,t));

                    // update min/max
                    if(s<minS)
                        minS = s;
                    if(t>maxS)
                        maxS = t;
                }
            }
        }

        // Compute endpoints intersections
        if(minS > tolerance && !isClosed_) // start
        {
            T va = vertices_.front();
            T ve = (*this)(tolerance);
            T vb = ve.lerp(2.0, va);
            for(int j=1; j<n-1; ++j)
            {
                T vc = (*this)[j];
                T vd = (*this)[j+1];

                bool doIntersect = intersects(va, vb, vc, vd, u, v);
                if(doIntersect)
                {
                    double t = (1-v)*arclengths_[j] + v*arclengths_[j+1];
                    res.push_back(Intersection(0,t));

                    // update min/max
                    if(t>maxS)
                        maxS = t;
                }
            }
        }
        if(maxS < l-tolerance && !isClosed_) // end
        {
            T va = vertices_.back();
            T ve = (*this)(l-tolerance);
            T vb = ve.lerp(2.0, va);
            for(int j=0; j<n-3; ++j)
            {
                T vc = (*this)[j];
                T vd = (*this)[j+1];

                bool doIntersect = intersects(va, vb, vc, vd, u, v);
                if(doIntersect)
                {
                    double t = (1-v)*arclengths_[j] + v*arclengths_[j+1];
                    res.push_back(Intersection(t,l));
                }
            }
        }

        return res;
    }


    // Split the curve: guarantees that res.size() = splitValues.size() - 1
    // Input: split values. e.g : [0, 230, l]
    // Output: a list of curves: [subcurve(0->230) , subcurve(230->l)]
    // splitValues must have a size>=2 to return a non-empty vector
    // if the curve is a loop, the values are expected to be in the range 0..l except the last one
    // and be like: [ s1 s2 s3 ... sn s1+l ]
    std::vector< Curve<T>,Eigen::aligned_allocator<Curve<T> >  > split(const std::vector<double> & splitValues) const
    {
        using std::cout;
        using std::endl;


        //cout << "\n\n\n ########### Entering split #########" << endl;
        //cout << "Split values:";
        //for(auto s: splitValues)
        //    std::cout << " " << s;
        //cout << endl;

        // for numerical corner cases: when one split value is exactly a vertex
        //double epsilon = 1e-5;

        // get number of split required.
        int nSplitValues = splitValues.size();

        // trivial cases
        if(nSplitValues < 2)
        {
            //cout << "not enough split values" << endl;
            return std::vector< Curve<T>,Eigen::aligned_allocator<Curve<T> >  >();
        }

        int n = size();
        if(n == 0)
        {
            //cout << "curve is null" << endl;
            return std::vector< Curve<T>,Eigen::aligned_allocator<Curve<T> >  >(nSplitValues+1);
        }

        // Now, we know the curve is non-null, and that there is at least one split
        precomputeArclengths_();

        //cout << "Curve to split:";
        //for(auto s: vertices_)
        //    std::cout << " (" << s.x() << "," << s.y() << "," << s.width() << ") ";
        //cout << endl;
        //cout << "arclengths:";
        //for(auto s: arclengths_)
        //    std::cout << " " << s;
        //cout << endl;



        int i = 0;
        T lastVertexOfLastCurve;
        // find first vertex
        // skip all vertices 0, 1, .., i, i+1, i+2, ...., i+k with an arclength strictly less than splitValues.front()
        //cout << "arclengths_[i] to be tested = arclength[" << 0 << "] = " << arclengths_[0] << endl;
        while(i<n && arclengths_[i] < splitValues.front() /*+ epsilon*/)
        {
            i++;
            //cout << "arclengths_[i] to be tested = arclength[" << i << "] = " << arclengths_[i] << endl;
        }
        //cout << "now, i = " << i << endl;
        // compute first vertex
        if(i == 0)
        {
            lastVertexOfLastCurve = start();
            i++;
            //cout << "we had i=0, hence lastvertex is first vertex" << endl;
        }
        else if(i<n)
        {
            double  u = (splitValues.front() - arclengths_[i-1]) / (arclengths_[i] - arclengths_[i-1]);
            lastVertexOfLastCurve = vertices_[i-1].lerp(u, vertices_[i]);
            //cout << "we had i!=0, so we computed an interpolated vertex : ("
            //     << lastVertexOfLastCurve.x() << ","
            //     << lastVertexOfLastCurve.y() << ","
            //     << lastVertexOfLastCurve.width() << endl;

            // special case when created vertex too close from next one: skip next one
            // this case happens when the requested splitValue is exactly the arclength of one of
            // the vertex
            if(u>0.99)
            {
                i++;
            }
        }
        else
        {
            lastVertexOfLastCurve = end();
            //cout << "i was >= n, hence lastvertex is last vertex" << endl;
        }

        // to handle loop
        bool hasLooped = false;

        int splitIndex = 1;
        std::vector< Curve<T>,Eigen::aligned_allocator<Curve<T> >  > res;
        // loop invariant: splitIndex-1 == res.size()
        //cout << "entering loop over split values" << endl;
        while(splitIndex < nSplitValues)
        {
            //cout << "\n ## starting a new sub-curve ##" << endl;

            //cout << "starting a new sub-curve" << endl;

            //cout << "splitIndex = " << splitIndex << endl;

            // extract a sub-curve from vertex i included to arclength "splitValue"
            double splitValue = splitValues[splitIndex];
            //cout << "splitValue = " << splitValue << endl;

            Curve curve(ds_);
            if(hasLooped)
            {
                splitValue -= length();
                //cout << "but we've looped so splitValue = " << splitIndex << endl;
            }

            // add last vertex of previous sub-curve, if any
            curve.vertices_.push_back(lastVertexOfLastCurve);
            curve.arclengths_.push_back(0);
            //cout << "adding the first vertex: ("
            //     << curve.vertices_.back().x() << ","
            //     << curve.vertices_.back().y() << ","
            //     << curve.vertices_.back().width() << ")  with arclength = "
            //     << curve.arclengths_.back() << endl;

            //cout << "entering loop to add all vertices" << endl;
            // add all vertices i, i+1, i+2, ...., i+k with an arclength strictly less than splitValue
            while( (i>=n) || (arclengths_[i] < splitValue/* - epsilon*/) ) // basically: while(arclengths_[i] < splitValue)
                                                             // but we do not want to leave the loop when arclengths_[i]
                                                             // is ill-defined, and instead handle it, correct it, and resume the loop
            {
                //cout << "new loop body execution: i = " << i << " and n = " << n << endl;
                if(i<n) // normal case
                {
                    curve.vertices_.push_back(vertices_[i]);
                    curve.arclengths_.push_back(arclengths_[i]-splitValues[splitIndex-1]);
                    //cout << "added vertex: ("
                    //     << curve.vertices_.back().x() << ","
                    //     << curve.vertices_.back().y() << ","
                    //     << curve.vertices_.back().width() << ")  with arclength = "
                    //     << curve.arclengths_.back() << endl;
                    i++;
                }
                else // case where arclengths_[i] is ill-defined because we reached end of loop
                {
                    //cout << "woops, we've reached i = n" << endl;
                    if(isClosed_ && !hasLooped)
                    {
                        //cout << "that's because this curve is a loop!" << endl;
                        hasLooped = true;
                        i = 1; // not 0 because the last vertex is equal to the first, we don't want to add it twice
                        splitValue -= length();
                        //cout << "the new split value is: " << splitValue << endl;
                    }
                    else
                    {
                        //cout << "well, then we just leave the loop :-)" << endl;
                        break;
                    }
                }
            }
            //cout << "leaving loop iterating over vertices" << endl;
            //cout << "i = " << i << endl;
            // here, (arclengths_[i-1] < splitValue and arclengths_[i] >= splitValue) or (i==n)

            // add new vertex inbetween i-1 and i
            if( (0<i-1) && (i<n) ) // question: why not i-1 >= 0 instead ?
            {
                // here, maybe should check if that's necessary or not
                // I think I must check that u is > eps

                //cout << "let's add one more vertex" << endl;

                double  u = (splitValue - arclengths_[i-1]) / (arclengths_[i] - arclengths_[i-1]);


                // same precaution the other way around:
                // special case when created vertex too close from previous one:then don't add it
                if(u>0.01)
                {
                    curve.vertices_.push_back(vertices_[i-1].lerp(u, vertices_[i]));
                    curve.arclengths_.push_back(splitValue-splitValues[splitIndex-1]);

                    //cout << "splitValue = " << splitValue
                    //     << "    arclengths_[i-1] = " << arclengths_[i-1]
                    //     << "    arclengths_[i] = " << arclengths_[i]
                    //     << "    u = " << u
                    //     << "    vertex = ("
                    //     << curve.vertices_.back().x() << ","
                    //     << curve.vertices_.back().y() << ","
                    //     << curve.vertices_.back().width() << ")  with arclength = "
                    //     << curve.arclengths_.back() << endl;

                    // special case when created vertex too close from next one: skip next one
                    // this case happens when the requested splitValue is exactly the arclength of one of
                    // the vertex
                    if(u>0.99)
                    {
                        i++;
                    }
                }
                else
                {
                    // nothing to do, the next vertex will naturally be added
                }
            }

            // handle special case
            if(isClosed_)
                curve.setDirtyArclengths_();

            // add the curve to the result
            res.push_back(curve);
            lastVertexOfLastCurve = res.back().end();
            //cout << "added the new curve to split curve list, its last vertex is: "
            //     << lastVertexOfLastCurve.x() << ","
            //     << lastVertexOfLastCurve.y() << ","
            //     << lastVertexOfLastCurve.width() << endl;

            // increment split index
            splitIndex++;
        }
        // here:  splitIndex == nSplitValues && splitIndex-1 == res.size()
        // hence  res.size() == nSplitValues - 1

        //returns
        return res;
    }

    // ----- Curve geometry editing -----

    void setEndPoints(const T & newStart, const T & newEnd)
    {
        int n = size();
        if(n < 3)
        {
            vertices_.clear();
            vertices_.push_back(newStart);
            vertices_.push_back(newEnd);
        }
        else
        {
            precomputeArclengths_();

            // this method could be replaced using lerp only, but it would make it
            // much much less efficient and readable
            T dStart = newStart - vertices_.front();
            T dEnd = newEnd - vertices_.back();

            double l = length();
            assert(l>0);
            for(int i=0; i<n; ++i)
            {
                double a = arclengths_[i] / l;
                vertices_[i] = vertices_[i] + dStart.lerp(a, dEnd);
            }
        }

        resample(true);
    }

private:
    // Sampled curve: the one that is exposed to the user
    std::vector<T,Eigen::aligned_allocator<T> > vertices_;

    // Arc-length precomputation
    mutable std::vector<double> arclengths_;
    mutable bool dirtyArclengths_;

    // If treated as a loop
    bool isClosed_;

    // ---- Sketching: curve neatening algorithm  ----

    bool sketchInProgress_;
    void pushFirstVertex_(const T & vertex)
    {
        arclengths_.push_back(0);
        vertices_.push_back(vertex);
    }
    void pushVertex_(const T & vertex)
    {
        double d = vertices_.back().distanceTo(vertex);
        if(d>0)
        {
            arclengths_.push_back(arclengths_.back() + d);
            vertices_.push_back(vertex);
        }
    }
    T interpolatedVertex_(double s) const // size must be > 1
    {
        precomputeArclengths_();

        int i = 0;
        int j = vertices_.size()-1;
        double si = arclengths_[i];
        double sj = arclengths_[j];

        int k;
        double sk, u;

        while( j-i > 1)
        {
            // compute an index hopefully close to s
            u = (s - si) / (sj - si);
            k = std::floor( (1-u)*i + u*j );

            // make sure i < k < j
            k = std::min(j-1, std::max(i+1, k));

            // recurse
            sk = arclengths_[k];
            if(sk > s)
            {
                j = k;
                sj = sk;
            }
            else
            {
                i = k;
                si = sk;
            }
        }

        // Now: j = i+1
        u = (s - si) / (sj - si);
        return vertices_[i].lerp(u,vertices_[j]);
    }



    // parameters
    int N_;

    // temp variables
    std::vector<T,Eigen::aligned_allocator<T> > qTemp_; // vertices only valid until next mouse sample
                                   // the ones before are final and stored in q_
    //QList<double> qtTemp_; // same but for time information // NOT needed now: part of T
    double lastFinalS_; // value s such that phi(s) = q_.last()

    // raw input from mouse
    struct Input
    {
        Input(const T & p, double s) : p(p), s(s) {}
        T p;
        double s;
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };
    std::vector<Input,Eigen::aligned_allocator<Input> > p_;

    // fit a smooth curve to a subpart of the raw mouse input
    class Fitter
    {
    public:
        // the fitting computation must be implemented in derived constructors.
        //
        // the N input points from p[j] to p[j+N-1] (guaranteed to exist)
        // is the local part of the curve that should be fit.
        //
        Fitter(const std::vector<Input,Eigen::aligned_allocator<Input> > & p, int j, int N, double ds) :
            p_(p), j_(j), N_(N), ds_(ds) {}
        virtual ~Fitter() {}

        // eval() must be implemented in derived classes
        //
        // the fitting curve C must:
        //    - be a continuous curve parameterized from startS to endS
        //    - verify C(startS) = startP and C(endS) = endP
        //    - have a precision of at least ds
        //
        // it is better, but not necessary, if the parameterization is as
        // close as possible to a (scaled) arc-length parameterization.
        //
        // it is better, but not necessary, if the length of the curve is
        // as close as possible to endS - startS.
        //
        // if useful, the method u_(s) can be used to map [startS, endS] to
        // [0,1], if a this parameterization range is easier to handle
        //
        virtual Eigen::Vector2d eval(double s) const=0;

        // weight that should be use for this curve at this parameter to
        // blend the different overlapping local fits together
        //
        // this method can be overriden in derived class if custom blending
        // weights are desired (but the default ones should be fine)
        virtual double w(double s) const
        {
            double u = u_(s);
            return u*u*(1-u)*(1-u);
        }

        // convenient inline methods to make code easier to write and read
        inline Eigen::Vector2d p(int i) const {return Eigen::Vector2d(p_[i].p.x(), p_[i].p.y()) ;}
        inline double s(int i) const {return p_[i].s;}
        inline double startS() const {return s(j_);}
        inline double endS() const {return s(j_+N_-1);}
        inline Eigen::Vector2d startP() const {return p(j_);}
        inline Eigen::Vector2d endP() const {return p(j_+N_-1);}

    protected:
        // maps [startS, endS] to [0,1]
        double u_(double s) const {return (s-startS())/(endS()-startS());}

        // reference to the whole raw mouse input
        const std::vector<Input,Eigen::aligned_allocator<Input> > & p_;

        // local range of vertices to fit: the N points p_[j] to p_[j+N-1]
        int j_;
        int N_;

        // required precision, to be used if derived classes need sampling
        double ds_;
    };

    // List storing all the local fitting
    std::vector<Fitter*> fits_;
    void clearFits_()
    {
        for(Fitter * f : fits_)
            delete f;
        fits_.clear();
    }

    // Blend overlapping fitting together
    int lastFittingInvolved_i; // not initialized, but it's ok
    int lastFittingInvolved_(double s)
    {
        int i = lastFittingInvolved_i; // make it more readable
        if(i<0 || (unsigned int) i >= p_.size())
            i = 0;
        while(i-1>=0 && s<p_[i-1].s)
            i--;
        while((unsigned int)i+1<p_.size() && s>=p_[i+1].s)
            i++;
        return i;
    }

    T phi_(double s, Fitter * useSingleFit = 0)
    {
        // compute pos
        Eigen::Vector2d pos(0,0);
        if(useSingleFit)
        {
            pos = useSingleFit->eval(s);
        }
        else
        {
            int i=lastFittingInvolved_(s);
            int startJ = std::max(0,i-N_+2);
            int endJ = std::min(i,(int)p_.size()-N_);
            if(startJ==endJ)
            {
                pos = fits_[startJ]->eval(s);
            }
            else
            {
                double sumW = 0;
                for(int j=startJ; j<=endJ; j++)
                {
                    double w = fits_[j]->w(s);
                    Eigen::Vector2d q = fits_[j]->eval(s);
                    pos = pos + w * q;
                    sumW += w;
                }
                pos = (1/sumW) * pos;
            }
        }

        // get other data as linear interpolation of raw mouse input (no smoothing)
        T res;
        if(p_.size() == 0)
        {
            // nothing, keep default-constructed data
        }
        else if(p_.size() == 1)
        {
            res = p_[0].p;
        }
        else
        {
            int i = p_.size()-1;
            while(p_[i-1].s > s && i>1)
                i--;

            // now, we have p_[i-1].s <= s < p_[i]
            // note: we know that s2 - s1 > 0 since we rejected before
            // the input that did not satisfy this condition
            double s1 = p_[i-1].s;
            double s2 = p_[i].s;
            double u = (s-s1)/(s2-s1);
            res = p_[i-1].p.lerp(u, p_[i].p);
        }

        // change position
        res.setX(pos[0]);
        res.setY(pos[1]);

        return res;
    }

    //double phit_(double s); // useless now, part of T

    // Different fitting algorithms that can be used
    // Note: only smooth the x and y coordinates. Could use higher dimensional
    //       bezier to also smooth other data, but not done by design
    class CubicBezierFitter: public Fitter
    {
    public:
        CubicBezierFitter(const std::vector<Input,Eigen::aligned_allocator<Input> > & p, int j, int N, double ds) :
            Fitter(p,j,N, ds)
        {

            assert(N>=2);

            // --- Fit a bezier curve to the input points ---

            // end points
            P0_ = this->startP();
            P3_ = this->endP();

            // handle special cases:
            if(N==2)
            {
                // linear:
                P1_ = 2./3. *  P0_ + 1./3. * P3_;
                P2_ = 1./3. *  P0_ + 2./3. * P3_;
            }
            else if(N==3)
            {
                // At*A is also singular in this case
                // fit a dum quadratic (todo: smth better)
                P1_ = this->p(j+1);
                P2_ = this->p(j+1);
            }
            else
            {

                // create linear system
                Eigen::MatrixXd A(2*(this->N_-2),4);
                Eigen::VectorXd B(2*(this->N_-2));
                double Ax = this->startP()[0];
                double Ay = this->startP()[1];
                double Dx = this->endP()[0];
                double Dy = this->endP()[1];
                for(int i=1; i<this->N_-1; i++)
                {
                    double Ui = this->u_(this->s(i+this->j_));
                    double OneMinusUi = 1-Ui;
                    double Ui2 = Ui*Ui;
                    double OneMinusUi2 = OneMinusUi*OneMinusUi;
                    double Ui3 = Ui2*Ui;
                    double OneMinusUi3 = OneMinusUi2*OneMinusUi;
                    double ThreeOneMinusUi2Ui = 3 * OneMinusUi2 * Ui;
                    double ThreeOneMinusUiUi2 = 3 * OneMinusUi * Ui2;

                    int TwoTimesIMinus1 = 2*(i-1);
                    int TwoTimesIMinus1Plus1 = 2*(i-1)+1;

                    A(TwoTimesIMinus1, 0) = ThreeOneMinusUi2Ui;
                    A(TwoTimesIMinus1, 1) = 0;
                    A(TwoTimesIMinus1, 2) = ThreeOneMinusUiUi2;
                    A(TwoTimesIMinus1, 3) = 0;

                    A(TwoTimesIMinus1Plus1, 0) = 0;
                    A(TwoTimesIMinus1Plus1, 1) = ThreeOneMinusUi2Ui;
                    A(TwoTimesIMinus1Plus1, 2) = 0;
                    A(TwoTimesIMinus1Plus1, 3) = ThreeOneMinusUiUi2;

                    B(TwoTimesIMinus1) = this->p(i+this->j_)[0] - OneMinusUi3 * Ax - Ui3 * Dx;
                    B(TwoTimesIMinus1Plus1) = this->p(i+this->j_)[1] - OneMinusUi3 * Ay - Ui3 * Dy;
                }

                // solve it
                //Eigen::VectorXd X = (A.transpose() * A).inverse() * A.transpose() * B;
                // For some reason above code breaks with Eigen 3.2, need to split it
                Eigen::MatrixXd M = (A.transpose() * A).inverse();
                M *= A.transpose();
                Eigen::VectorXd X = M * B;

                P1_ = Eigen::Vector2d(X(0),X(1));
                P2_ = Eigen::Vector2d(X(2),X(3));
            }

            // --- Compute an approximate uniform parameterization ---

            for(double u=0; u<1; u+=0.75*this->ds_/der(u).norm())
            {
                sampling_ << this->pos(u);
            }
                sampling_ << P3_;
            // => sampling_.size() >= 2
        }

        Eigen::Vector2d eval(double s) const
        {
            double u = this->u_(s);

            // i = iQ + iR with iQ \in N and iR \in [0,1)
            double i = u * (sampling_.size()-1);
            int iQ = std::floor(i);
            double iR = i - iQ;
            if(iQ<0)
                iQ = 0;
            else if( (unsigned int) iQ >= sampling_.size())
                iQ = sampling_.size()-1;


            if( (unsigned int) iQ == sampling_.size()-1)
                return sampling_.back(); // = D_
            else
                return (1-iR)*sampling_[iQ] + iR*sampling_[iQ+1];
        }

    private:
        Eigen::Vector2d pos(double u)
        {
            return (1-u)*(1-u)*(1-u)*P0_ + 3*(1-u)*(1-u)*u*P1_ + 3*(1-u)*u*u*P2_ + u*u*u*P3_;
        }

        Eigen::Vector2d der(double u)
        {
            return 3*( (1-u)*(1-u)*(P1_-P0_) + 2*(1-u)*u*(P2_-P1_) + u*u*(P3_-P2_) );
        }

        Eigen::Vector2d P0_, P1_, P2_, P3_;
        std::vector<Eigen::Vector2d,Eigen::aligned_allocator<Eigen::Vector2d> > sampling_;
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };

    class QuarticBezierFitter: public Fitter
    {
    public:
        QuarticBezierFitter(const std::vector<Input,Eigen::aligned_allocator<Input> > & p, int j, int N, double ds) :
            Fitter(p,j,N, ds)
        {
            assert(N>=2);

            // --- Fit a quartic (degree four) bezier curve to the input points ---

            // end points
            P0_ = this->startP();
            P4_ = this->endP();

            if(N==2)
            {
                P1_ = 3./4. *  P0_ + 1./4. * P4_;
                P2_ = 2./4. *  P0_ + 2./4. * P4_;
                P3_ = 1./4. *  P0_ + 3./4. * P4_;
            }
            else if(N==3)
            {
                P2_ = p(j+1);
                P1_ = 0.5 * (P0_ + P2_);
                P3_ = 0.5 * (P2_ + P4_);
            }
            else if(N==4)
            {
                P1_ = p(j+1);
                P3_ = p(j+2);
                P2_ = 0.5 * (P1_ + P3_);
            }
            else
            {

                // create linear system
                Eigen::MatrixXd A(2*(this->N_-2),6);
                Eigen::VectorXd B(2*(this->N_-2));
                for(int i=1; i<this->N_-1; i++)
                {
                    int TwoTimesIMinus1 = 2*(i-1);
                    int TwoTimesIMinus1Plus1 = 2*(i-1)+1;

                    double u = u_(s(i+this->j_));

                    A(TwoTimesIMinus1, 0) = 4*(1-u)*(1-u)*(1-u)*u;
                    A(TwoTimesIMinus1, 1) = 0;
                    A(TwoTimesIMinus1, 2) = 6*(1-u)*(1-u)*u*u;
                    A(TwoTimesIMinus1, 3) = 0;
                    A(TwoTimesIMinus1, 4) = 4*(1-u)*u*u*u;
                    A(TwoTimesIMinus1, 5) = 0;

                    A(TwoTimesIMinus1Plus1, 0) = 0;
                    A(TwoTimesIMinus1Plus1, 1) = 4*(1-u)*(1-u)*(1-u)*u;
                    A(TwoTimesIMinus1Plus1, 2) = 0;
                    A(TwoTimesIMinus1Plus1, 3) = 6*(1-u)*(1-u)*u*u;
                    A(TwoTimesIMinus1Plus1, 4) = 0;
                    A(TwoTimesIMinus1Plus1, 5) = 4*(1-u)*u*u*u;

                    B(TwoTimesIMinus1) = p(i+this->j_)[0] - (1-u)*(1-u)*(1-u)*(1-u) * P0_[0] - u*u*u*u * P4_[0];
                    B(TwoTimesIMinus1Plus1) = p(i+this->j_)[1] - (1-u)*(1-u)*(1-u)*(1-u) * P0_[1] - u*u*u*u * P4_[1];
                }

                // solve it
                Eigen::MatrixXd M = (A.transpose() * A).inverse();
                M *= A.transpose();
                Eigen::VectorXd X = M * B;
                P1_ = Eigen::Vector2d(X(0),X(1));
                P2_ = Eigen::Vector2d(X(2),X(3));
                P3_ = Eigen::Vector2d(X(4),X(5));
            }

            // --- Compute an approximate uniform parameterization ---

            for(double u=0; u<1; u+=0.75*this->ds_/der(u).norm())
            {
                sampling_ << this->pos(u);
            }
            sampling_ << P4_;
            // => sampling_.size() >= 2
        }

        Eigen::Vector2d eval(double s) const
        {
            double u = this->u_(s);

            // i = iQ + iR with iQ \in N and iR \in [0,1)
            double i = u * (sampling_.size()-1);
            int iQ = std::floor(i);
            double iR = i - iQ;
            if(iQ<0)
                iQ = 0;
            else if(iQ>=sampling_.size())
                iQ = sampling_.size()-1;
            if(iQ==sampling_.size()-1)
                return sampling_.back(); // = D_
            else
                return (1-iR)*sampling_[iQ] + iR*sampling_[iQ+1];
        }

    private:
        Eigen::Vector2d pos(double u)
        {
            return (1-u)*(1-u)*(1-u)*(1-u)*P0_ + 4*(1-u)*(1-u)*(1-u)*u*P1_ + 6*(1-u)*(1-u)*u*u*P2_ + 4*(1-u)*u*u*u*P3_ + u*u*u*u*P4_;
        }
        Eigen::Vector2d der(double u)
        {
            return 4*( (1-u)*(1-u)*(1-u)*(P1_-P0_) + 3*(1-u)*(1-u)*u*(P2_-P1_) + 3*(1-u)*u*u*(P3_-P2_) + u*u*u*(P4_-P3_) );
        }


        Eigen::Vector2d P0_, P1_, P2_, P3_, P4_;
        std::vector<Eigen::Vector2d,Eigen::aligned_allocator<Eigen::Vector2d> > sampling_;
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };

    // Current fitting algorithm to use
    enum FitterType
    {
        CUBIC_BEZIER_FITTER,
        QUARTIC_BEZIER_FITTER,
        CLOTHOID_FITTER
    };
    FitterType fitterType_;

    // Create a new Fitter object (call the appropriate derived constructor)
    // It is caller's responsability to call "delete" to free the memory
    static Fitter * fitter(FitterType /*type*/, const std::vector<Input,Eigen::aligned_allocator<Input> > & p, int j, int N, double ds)
    {
        /*
        switch(type)
        {
        case CUBIC_BEZIER_FITTER:
            return new CubicBezierFitter(p, j, N, ds);
        case QUARTIC_BEZIER_FITTER:
            return new QuarticBezierFitter(p, j, N, ds);
        }
        */
        return new CubicBezierFitter(p, j, N, ds);
    }

    // Sampling
    double ds_;
    double lastDs_;
    void setDirtyArclengths_()   const { dirtyArclengths_ = true; }
    void precomputeArclengths_() const
    {
        if(!dirtyArclengths_)
            return;

        int n = size();
        assert(n>0);

        arclengths_.resize(n);
        arclengths_[0] = 0;
        for(int i=1; i<n; ++i)
            arclengths_[i] = arclengths_[i-1] + vertices_[i-1].distanceTo(vertices_[i]);

        dirtyArclengths_ = false;
    }
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

} // end namespace SculptCurve

#endif

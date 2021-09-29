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

#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

/// \class BoundingBox
/// A bounding box represents an axis-aligned rectangle, possibly
/// empty, possibly degenerate (=zero area), possibly infinite.
///
/// A bounding box is stored as 4 doubles: xMin, xMax, yMin, and yMax. Each of
/// these values may be infinite, and xMin=xMax and/or yMin=yMax is allowed.
///
/// Every non-empty bounding box satisfies:
///     xMin <= xMax
///     yMin <= yMax
///
///
/// EMPTY BOUNDING BOX
/// ------------------
///
/// The empty bounding box {} is a special case to represent an empty set.
/// It is stored as:
///     xMin = + infinity
///     yMin = + infinity
///     xMax = - infinity
///     yMax = - infinity
///
///
/// WIDTH AND HEIGHT
/// ----------------
///
/// Width and height are defined by:
///   - (xMax-xMin) and (yMax-yMin) for non-empty bounding boxes
///   -     0.0     and     0.0     for the empty bounding box
///
/// They always satisfy width >= 0 and height >=0.
///
///
/// DEGENERATE BOUNDING BOXES
/// -------------------------
///
/// A degenerate bounding box is defined as a bounding box whose width or
/// height is equal to zero. For non-empty bounding boxes, this is equivalent
/// to xMin = xMax, yMin = yMax, or both.
///
/// The empty bounding box is degenerate, but not all degenerate bounding boxes
/// are empty.
///
/// One example of degenerate bounding box is a single point in space:
///     xMin = x
///     yMin = y
///     xMax = x
///     yMax = y
///
/// Another example of degenerate bounding box is an axis-aligned segment:
///     xMin = x1
///     yMin = y
///     xMax = x2
///     yMax = y
///
/// None of the above are empty, even when x, x1, x2, and/or y are equal to
/// +infinity or -infinity.
///
///
/// INFINITE BOUNDING BOXES
/// -----------------------
///
/// An infinite bounding box is defined as a bounding box whose width or
/// height is equal to +infinity.
///
/// The empty bounding box is not infinite.
///
/// A bounding box can be both degenerate and infinite, i.e. has a zero-width
/// and infinite-height, or vice-versa.
///
///
/// PROPER BOUNDING BOXES
/// ---------------------
///
/// A proper bounding box is defined as a bounding box which is non-degenerate
/// and non-infinite.
///
/// Being proper is equivalent to:
///     xMin < xMax
///     yMin < yMax
///     xMin, xMax, yMin, yMax are neither -infinity nor +infinity
///
/// The empty bounding box is not proper.
///
///
/// UNION AND INTERSECTION
/// ----------------------
///
/// The union of two bounding boxes B1 and B2 is defined as the smallest
/// bounding box that contains both B1 and B2.
///
/// The intersection of two bounding boxes B1 and B2 is defined as the largest
/// bounding box which is contained in both B1 and B2.
///
/// The union of the empty bounding box {} with any bounding box B is always
/// equal to B. The intersection of the empty bounding box {} with any bounding
/// box B is always equal to {}.
///
/// However, the union between a degenerate bounding box B' and a bounding box
/// B is generally not equal to B, as illustrated below:
///
///     ...................
///     : : B'=segment    : union(B,B') != B even though B' is degenerate
///     : :               :
///     : :      ........ :
///     : :    B :      : :
///     :        :      : :
///     :        :......: :
///     :.................:
///
/// Also, the union of two degenerate boxes is generally not degenerate:
///
///      ....................
///      : : B'=segment     : union(B,B') is non-degenerate even though
///      : :                : both B and B' are
///      : :                :
///      : :                :
///      :                  :
///      :        B=point * :
///      :...................
///     
/// The intersection between a degenerate bounding box B' and any bounding box
/// B is always degenerate:
/// 
///     
///             : B'=segment
///             :
///     ........|.....................................
///     :       | intersection(B,B') != {}           : B
///     :       | even though B' is degenerate       :
///     :       |                                    :
///     :.......|....................................:
///             :
///             :
///             :
///
/// Union preserves properness, non-degeneracy, finiteness, and infiniteness:
///     - The union between two proper bounding boxes is proper.
///     - The union between two non-degenerate bounding boxes is non-degenerate.
///     - The union between two finite bounding boxes is finite.
///     - The union between two infinite bounding boxes is infinite.
///
/// Union does not generally preserve non-properness and degeneracy:
///     - The union between two non-proper bounding boxes may be proper.
///     - The union between two degenerate bounding boxes may be non-degenerate.
///     
/// Intersection preserves degeneracy and finiteness:
///     - The intersection between two degenerate bounding boxes is degenerate.
///     - The intersection between two finite bounding boxes is finite.
///     
/// Intersection does not generally preserve properness, non-properness,
/// non-degeneracy and infiniteness:
///     - The intersection between two proper bounding boxes may be non-proper.
///     - The intersection between two non-proper bounding boxes may be proper.
///     - The intersection between two non-degenerate bounding boxes may be degenerate.
///     - The intersection between two infinite bounding boxes may be finite.
/// 
///
/// AREA OF BOUNDING BOXES
/// ----------------------
///
/// The area of a bounding box is defined by:
///     - zero for degenerate bounding boxes (including when empty or infinite)
///     - width*height otherwise
///
/// Therefore:
///     - The area is always >= 0
///     - The area of the empty bounding box is 0
///     - The area of any degenerate bounding box is 0
///     - The area of any proper bounding box is > 0 and non-infinite
///     - The area of any non-degenerate, infinite bounding box is +infinity
///
/// Note that the area of a bounding box both degenerate and infinite is 0,
/// which is different from width*height = 0*inf = NaN. All public methods of
/// the BoundingBox class are guaranteed to never return NaN.
///
///
/// MATHEMATICAL NOTES
/// ------------------
///
/// For the mathematician reader, a bounding box is formally defined as the
/// cross product of two closed intervals of \R, where \R refers to the
/// extended real number line, commonly called "R bar" (R U {-inf, +inf}), and
/// where a closed interval of \R is any closed convex set of \R, i.e.:
///   - the empty set {}, or
///   - [x1, x2] with x1 in \R, x2 in \R, and x1 <= x2

#include "VAC/vpaint_global.h"

namespace VectorAnimationComplex
{
    
class Q_VPAINT_EXPORT BoundingBox
{      
public:
    // Empty bounding box
    BoundingBox();

    // Single-point bounding box at position (x,y)
    BoundingBox(double x, double y);
    
    // Non-empty bounding box specified by its boundaries.
    // It is safe to call this constructor with either x1==x2, x1<x2, or x2<x1.
    BoundingBox(double x1, double x2, double y1, double y2);
    
    // Check emptyness, degeneracy, infiniteness, and properness
    bool isEmpty()      const;
    bool isDegenerate() const;
    bool isInfinite()   const;
    bool isProper()     const;

    // Get boundaries
    inline const double & xMin() const { return xMin_; }
    inline const double & xMax() const { return xMax_; }
    inline const double & yMin() const { return yMin_; }
    inline const double & yMax() const { return yMax_; }
            
    // Compute mid-points (0 if empty, or if min = -infinity and max = +infinity)
    double xMid()  const;
    double yMid() const;

    // Compute width, height, and area
    double width()  const;
    double height() const;
    double area()   const;

    // Compute union and intersection
    BoundingBox united      (const BoundingBox & other) const;
    BoundingBox intersected (const BoundingBox & other) const;

    // In-place union and intersection
    void unite     (const BoundingBox & other);
    void intersect (const BoundingBox & other);

    // Returns whether the two bounding boxes intersect
    bool intersects(const BoundingBox & other) const;
    
    // Comparison operators
    bool operator==(const BoundingBox & other) const;
    bool operator!=(const BoundingBox & other) const;
    
private:
    // Data members
    double xMin_, xMax_, yMin_, yMax_;
};

}

#endif // BOUNDINGBOX_H

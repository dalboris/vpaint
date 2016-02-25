// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_CELLTYPE_H
#define OPENVAC_CELLTYPE_H

namespace OpenVAC
{

class CellType
{
public:
    // Enumeration of valid values
    static const short KeyCell;
    static const short InbetweenCell;

    static const short VertexCell;
    static const short EdgeCell;
    static const short FaceCell;

    static const short KeyVertex;
    static const short KeyEdge;
    static const short KeyFace;

    static const short InbetweenVertex;
    static const short InbetweenEdge;
    static const short InbetweenFace;

    // Constructor
    CellType(short type) : type_(type) {}

    // Copy constructor
    CellType(const CellType & other) : type_(other.type_) {}

    // Assignment operator
    CellType & operator=(const CellType & other)
    {
        type_ = other.type_;
        return *this;
    }

    // Comparison operators
    bool operator==(const CellType & other) { return type_ == other.type_; }
    bool operator!=(const CellType & other) { return type_ != other.type_; }

    // Sub-type checking
    bool isKeyCell()       const { return type_ & KeyCell;       }
    bool isInbetweenCell() const { return type_ & InbetweenCell; }
    bool isVertexCell()    const { return type_ & VertexCell;    }
    bool isEdgeCell()      const { return type_ & EdgeCell;      }
    bool isFaceCell()      const { return type_ & FaceCell;      }

private:
    short type_;
};

}

#endif // OPENVAC_CELLTYPE_H

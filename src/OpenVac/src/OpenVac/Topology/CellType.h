// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_CELLTYPE_H
#define OPENVAC_CELLTYPE_H

namespace OpenVac
{

// Enumeration of CellType values.
// The dummy template param is a hack to define static const values in header.
template <class Dummy> struct CellTypes_
{
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
    static const short value;
};

template<class Dummy> const short CellTypes_<Dummy>::KeyCell       = 0x00;
template<class Dummy> const short CellTypes_<Dummy>::InbetweenCell = 0x01;

template<class Dummy> const short CellTypes_<Dummy>::VertexCell    = 0x02;
template<class Dummy> const short CellTypes_<Dummy>::EdgeCell      = 0x04;
template<class Dummy> const short CellTypes_<Dummy>::FaceCell      = 0x08;

template<class Dummy> const short CellTypes_<Dummy>::KeyVertex = 0x02; // = KeyCell | VertexCell;
template<class Dummy> const short CellTypes_<Dummy>::KeyEdge   = 0x04; // = KeyCell | EdgeCell;
template<class Dummy> const short CellTypes_<Dummy>::KeyFace   = 0x08; // = KeyCell | FaceCell;

template<class Dummy> const short CellTypes_<Dummy>::InbetweenVertex = 0x03; // = InbetweenCell | VertexCell;
template<class Dummy> const short CellTypes_<Dummy>::InbetweenEdge   = 0x05; // = InbetweenCell | EdgeCell;
template<class Dummy> const short CellTypes_<Dummy>::InbetweenFace   = 0x09; // = InbetweenCell | FaceCell;

class CellType: public CellTypes_<void>
{
public:
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

// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include <OpenVAC/Topology/CellType.h>

namespace OpenVAC
{

const short CellType::KeyCell       = 0x00;
const short CellType::InbetweenCell = 0x01;

const short CellType::VertexCell    = 0x02;
const short CellType::EdgeCell      = 0x04;
const short CellType::FaceCell      = 0x08;

const short CellType::KeyVertex = KeyCell | VertexCell;
const short CellType::KeyEdge   = KeyCell | EdgeCell;
const short CellType::KeyFace   = KeyCell | FaceCell;

const short CellType::InbetweenVertex = InbetweenCell | VertexCell;
const short CellType::InbetweenEdge   = InbetweenCell | EdgeCell;
const short CellType::InbetweenFace   = InbetweenCell | FaceCell;

}

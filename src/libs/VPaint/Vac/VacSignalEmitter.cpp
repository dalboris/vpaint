// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "VacSignalEmitter.h"

#include "Vac/Vac.h"

VacSignalEmitter::VacSignalEmitter(Vac * vac) :
    vac_(vac)
{
}

VacSignalEmitter::~VacSignalEmitter()
{
}

Vac * VacSignalEmitter::vac() const
{
    return vac_;
}

void VacSignalEmitter::topologyChanged(const OpenVac::TopologyEditInfo & info)
{
    emit vac()->topologyChanged(info);
}

void VacSignalEmitter::geometryChanged(const OpenVac::CellHandleSet & affected)
{
    emit vac()->geometryChanged(affected);
}

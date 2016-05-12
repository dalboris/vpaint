// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VAC_H
#define VAC_H

#include "Core/DataObject.h"
#include "Vac/VacData.h"
#include "Vac/VacSignalEmitter.h"

class VacSignalEmitter;

/// \class Vac
/// \brief A DataObject subclass that represents a vector animation complex.
///
/// This class is a wrapper around a OpenVac::Vac, which emits a changed()
/// signal when modification are done to the underlying OpenVac::Vac.
///
/// Note: VacData is simply an alias for OpenVac::Vac.
///
class Vac: public DataObject<VacData>
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(Vac)

public:
    /// Constructs a Vac.
    ///
    Vac();

    /// Destructs this Vac.
    ///
    ~Vac();

signals:
    /// Signal emitted whenever the topology has changed.
    ///
    void topologyChanged(
            const OpenVac::CellIdSet & created,
            const OpenVac::CellIdSet & destroyed,
            const OpenVac::CellIdSet & affected);

    /// Signal emitted whenever the geometry has changed. Note that changing
    /// the geometry of a cell often affects the geometry of incident cells as
    /// well. For instance, sculpting a key edge affects the geometry of all
    /// inbetween edges that interpolate it, and of all key faces that it
    /// supports.
    ///
    /// The signal topologyChanged() is always followed by the signal
    /// geometryChanged(), but geometryChanged() may be sent individually.
    ///
    void geometryChanged(
            const OpenVac::CellHandleSet & affected);

private:
    VacSignalEmitter vacSignalEmitter_;
};

#endif // VAC_H

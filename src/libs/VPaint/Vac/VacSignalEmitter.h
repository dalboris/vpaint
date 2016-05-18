// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VACOBSERVER_H
#define VACOBSERVER_H

#include "OpenVac/Util/VacObserver.h"

#include <QObject>

class Vac;

/// \class VacSignalEmitter
/// \brief A class that emits Vac Qt signals from OpenVac notifications.
///
/// This class is intended for private use only. It is instantiated by Vac to
/// convert OpenVac notifications into Qt signals.
///
class VacSignalEmitter: public OpenVac::VacObserver
{
private:
    Q_DISABLE_COPY(VacSignalEmitter)

public:
    /// Constructs the VacSignalEmitter associated with the given \p vac, which
    /// must be a non-null valid pointer.
    ///
    VacSignalEmitter(Vac * vac);

    /// Destructs this VacSignalEmitter.
    ///
    ~VacSignalEmitter();

    /// Overrides OpenVac::VacObserver::topologyChanged().
    /// Emits the Qt signals Vac::topologyChanged().
    ///
    virtual void topologyChanged(const OpenVac::TopologyEditInfo & info);

    /// Overrides OpenVac::VacObserver::topologyChanged().
    /// Emits the Qt signals Vac::geometryChanged().
    ///
    virtual void geometryChanged(const OpenVac::CellHandleSet & affected);

    /// Returns the Vac this VacSignalEmitter is associated with.
    ///
    Vac * vac() const;

private:
    Vac * vac_;
};

#endif // VACOBSERVER_H


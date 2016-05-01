// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_IDSTOHANDLESCOPIER_H
#define OPENVAC_IDSTOHANDLESCOPIER_H

#include <OpenVac/Data/Util/CellDataCopier.h>
#include <OpenVac/Operators/Util/IdToHandleTranslator.h>

namespace OpenVac
{

/// \class IdsToHandlesCopier OpenVac/Operators/Util/IdsToHandlesCopier.h
/// \brief Class to copy data from CellData<Ids> into CellData<Handles>.
///
/// This class encapsulates a CellDataCopier and an IdToHandleTranslator
/// to provide a copy operation from CellData<Ids> to CellData<Handles>.
///
class IdsToHandlesCopier
{
private:
    IdToHandleTranslator translator_;
    CellDataCopier<Ids, Handles> copier_;

public:
    /// Constructs an IdsToHandlesCopier with the given Vac.
    IdsToHandlesCopier(const Vac * vac) :
        translator_(vac),
        copier_(&translator_)
    {
    }

    /// Copies the given CellData<Ids> into the given CellData<Handles>. Each
    /// cell ID in \p 'from' is translated into the cell handle of the VAC
    /// given in the constructor with the corresponding ID, or to an empty
    /// handle if no cell in the Vac has such ID.
    void copy(const CellData<Ids> & from, CellData<Handles> & to)
    {
        copier_.copy(from, to);
    }
};

} // end namespace OpenVac

#endif // OPENVAC_IDSTOHANDLESCOPIER_H

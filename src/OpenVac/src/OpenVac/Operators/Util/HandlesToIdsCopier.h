// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_HANDLESTOIDSCOPIER_H
#define OPENVAC_HANDLESTOIDSCOPIER_H

#include <OpenVac/Data/Util/CellDataCopier.h>
#include <OpenVac/Operators/Util/HandleToIdTranslator.h>

namespace OpenVac
{

/// \class HandlesToIdsCopier OpenVac/Operators/Util/HandlesToIdsCopier.h
/// \brief Class to copy data from CellData<Handles> into CellData<Ids>.
///
/// This class encapsulates a CellDataCopier and an HandleToIdTranslator
/// to provide a copy operation from CellData<Handles> to CellData<Ids>.
///
class HandlesToIdsCopier
{
private:
    HandleToIdTranslator translator_;
    CellDataCopier<Handles, Ids> copier_;

public:
    /// Constructs an HandlesToIdsCopier with the given Vac.
    HandlesToIdsCopier() :
        translator_(),
        copier_(&translator_)
    {
    }

    /// Copies the given CellData<Handles> into the given CellData<Ids>. Each
    /// cell handle in \p 'from' is translated into its ID, or to 0 if the
    /// handle is empty.
    void copy(const CellData<Handles> & from, CellData<Ids> & to)
    {
        copier_.copy(from, to);
    }
};

} // end namespace OpenVac

#endif // OPENVAC_HANDLESTOIDSCOPIER_H

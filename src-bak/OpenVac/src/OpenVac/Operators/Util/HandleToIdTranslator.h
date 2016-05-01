// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_HANDLETOIDTRANSLATOR_H
#define OPENVAC_HANDLETOIDTRANSLATOR_H

#include <OpenVac/Data/Util/CellRefTranslator.h>
#include <OpenVac/Topology/Util/Handles.h>
#include <OpenVac/Operators/Util/Ids.h>

namespace OpenVac
{

/// \class HandleToIdTranslator OpenVac/Operators/Util/HandleToIdTranslator.h
/// \brief Class to translate a cell handle into a cell ID.
///
/// Usage:
/// \code
/// Vac * vac = someVac();
/// CellHandle h = someHandle(vac);
/// HandleToIdTranslator handleToId;
/// CellId id = handleToId.translate(h);
/// \endcode
///
/// This class is meant to be used with CellDataCopier, who requires
/// a pointer to a CellRefTranslator. For most other usages, it is preferrable
/// to directly write the following equivalent but more readable code:
///
/// \code
/// Vac * vac = someVac();
/// CellHandle handle = someHandle(vac);
/// CellId id = h->id();
/// \endcode
///
class HandleToIdTranslator: public CellRefTranslator<Handles, Ids>
{
public:
    /// Constructs a translator translating a cell handle into a cell ID. Note
    /// that unlike IdToHandleTranslator, there is no need to pass a Vac in
    /// argument, since each cell already stores its ID.
    HandleToIdTranslator() {}

    #define OPENVAC_HANDLETOIDTRANSLATOR_DEFINE_TRANSLATE_(CellType) \
        /** Translates a CellType##Handle into its CellType##Id, or 0 if   */ \
        /** the handle is empty                                            */ \
        void translate(const CellType##Handle & handle, \
                             CellType##Id     & id    ) \
        { \
            id = handle ? handle->id() : 0; \
        }

    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_HANDLETOIDTRANSLATOR_DEFINE_TRANSLATE_)
};

} // end namespace OpenVac

#endif // OPENVAC_HANDLETOIDTRANSLATOR_H

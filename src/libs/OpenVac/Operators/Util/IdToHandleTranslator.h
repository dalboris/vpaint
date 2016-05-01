// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_IDTOHANDLETRANSLATOR_H
#define OPENVAC_IDTOHANDLETRANSLATOR_H

#include <OpenVac/Data/Util/CellRefTranslator.h>
#include <OpenVac/Topology/Util/Handles.h>
#include <OpenVac/Operators/Util/Ids.h>
#include <OpenVac/Vac.h>

namespace OpenVac
{

/// \class IdToHandleTranslator OpenVac/Operators/Util/IdToHandleTranslator.h
/// \brief Subclass of CellRefTranslator to translate a cell ID into a cell
/// handle.
///
/// An IdToHandleTranslator stores a pointer to the Vac the translated cells
/// belong to. This Vac is given as an argument to the constructor of
/// IdToHandleTranslator. It is the responsibility of the owner of the
/// IdToHandleTranslator to ensure that this given Vac outlive the
/// IdToHandleTranslator.
///
/// Usage:
/// \code
/// Vac * vac = someVac();
/// CellId id = someId();
/// IdToHandleTranslator idToHandle(vac);
/// CellHandle handle = idToHandle.translate(id);
/// \endcode
///
/// This class is meant to be used with CellDataCopier, who requires
/// a pointer to a CellRefTranslator. For most other usages, it is preferrable
/// to directly write the following equivalent but more readable code:
///
/// \code
/// Vac * vac = someVac();
/// CellId id = someId();
/// CellHandle handle = vac->cell(id);
/// \endcode
///
class IdToHandleTranslator: public CellRefTranslator<Ids, Handles>
{
private:
    const Vac * vac_;

public:
    /// Constructs a translator translating a cell ID into a cell handle of the
    /// given Vac. It is the responsibility of the owner of the constructed
    /// translator to ensure that the given Vac outlive the translator.
    IdToHandleTranslator(const Vac * vac) : vac_(vac) {}

    #define OPENVAC_IDTOHANDLETRANSLATOR_DEFINE_TRANSLATE_(CellType) \
        /** Translates a CellType##Id into a CellType##Handle. Assumes   */ \
        /** that the Vac passed as argument of the constructor is still  */ \
        /** alive. Returns an empty handle if no cell in the Vac has the */ \
        /** given \p id.                                                 */ \
        void translate(const CellType##Id     & id, \
                             CellType##Handle & handle) \
        { \
            handle = vac_->cell(id); \
        }

    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_IDTOHANDLETRANSLATOR_DEFINE_TRANSLATE_)
};

} // end namespace OpenVac

#endif // OPENVAC_IDTOHANDLETRANSLATOR_H

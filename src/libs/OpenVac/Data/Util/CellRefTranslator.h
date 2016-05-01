// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_CELLREFTRANSLATOR_H
#define OPENVAC_CELLREFTRANSLATOR_H

#include <OpenVac/Core/ForeachCellType.h>

namespace OpenVac
{

/// \class CellRefTranslator OpenVac/Data/Util/CellRefTranslator.h
/// \brief Abstract base class for classes converting a T::CellRef into a
/// U::CellRef
///
/// Subclass CellRefTranslator and implement translate() to define a class
/// whose instances are "cell ref translators", and know how to translate a
/// T::CellRef into a U::CellRef.
///
/// The reason that "translate" is not implemented as a templated free function
/// is that the translator object may need to have a state, for instance a
/// pointer to the Vac the cells belong to, to be able to the translation.
///
/// Instances of CellRefTranslator are meant to be passed as arguments to
/// the constructor of CellDataCopier.
///
template <class T, class U>
class CellRefTranslator
{
public:
    #define OPENVAC_CELLDATACOPIER_DECLARE_TRANSLATE_(CellType) \
        /** Translates a T::CellType##Ref into a U::CellType##Ref */ \
        virtual void translate( \
            const typename T::CellType##Ref & from, \
                  typename U::CellType##Ref & to  )=0;

    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_CELLDATACOPIER_DECLARE_TRANSLATE_)
};

} // end namespace OpenVac

#endif // OPENVAC_CELLREFTRANSLATOR_H

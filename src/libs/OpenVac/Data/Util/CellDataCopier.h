// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_CELLDATACOPIER_H
#define OPENVAC_CELLDATACOPIER_H

#include <OpenVac/Data/CellData.h>
#include <OpenVac/Data/Util/CellRefTranslator.h>
#include <OpenVac/Data/Util/CellDataCopierVisitor.h>

#include <cassert>

namespace OpenVac
{

/// \class CellDataCopier OpenVac/Data/Util/CellDataCopier.h
/// \brief Class to copy data from a CellData<T> into a CellData<U>.
///
/// As a reminder, the CellData classes are very dumb. They are merely
/// containers for the data, but have no knowledge of what the data means. It
/// can be meaningful data or meaningless data, CellData does not care. The
/// responsibility of CellData is just to hold the data, whatever data, which
/// may be in an intermediate state considered "invalid" from the perspective
/// of the Vac, but perfectly OK from the perspective of CellData.
///
/// Therefore, CellData does not prevent the use of its default copy
/// constructor, even though it would rarely make sense from the perspective of
/// Vac. So it is the responsibility of the Vac and the Operator classes to be
/// smarter and "do the right thing", but "doing the right thing" requires
/// context that CellData does not have.
///
/// For instance, when duplicating a cell within a given Vac, then using the
/// default copy constructor is perfectly fine. However, when copying a cell
/// from a Vac \p vac1 to another Vac \p vac2, then all the cell references
/// must be converted from references to cells of \p vac1 to references to
/// cells of \p vac2, process which we call a "reference translation".
///
/// Copying all the raw data from one CellData \p 'from' to another CellData \p
/// 'to', then converting all the references in \p 'from' to something
/// meaningful for \p 'to' is the job of CellDataCopier. Though, CellDataCopier
/// doesn't know "how" to translate the references: this is the job of the
/// CellRefTranslator passed as argument of the CellDataCopier constructor. In
/// other words, given one cell reference, a CellRefTranslator knows how to
/// translate it, and given one cell data with several references, a
/// CellDataCopier knows how to copy the raw data and ask the translator to
/// translate every reference in the data.
///
/// Note that the type of a "cell reference" is given by the template parameter
/// T in CellData<T>. For instance, T::KeyVertexRef is the type of a reference
/// to a key vertex. This type might be 'unsigned int' if references are stored
/// as IDs (as is the case in Operator classes), or CellHandle if references
/// are stored as handles (as is the case in the Vac). So when copying from a
/// CellData<T> to a CellData<U> with T != U, the copy constructor cannot even
/// be called since the classes are unrelated (in the inheritance sense). In
/// this case, you have no other choice than use a CellDataCopier, constructed
/// with a translator that knows how to translate a T::KeyVertexRef (e.g., an
/// unsigned int), into a U::KeyVertexRef (e.g., a KeyVertexHandle).
///
template <class T, class U>
class CellDataCopier
{
private:
    CellRefTranslator<T, U> * translator_;

public:
    /// Construct a CellDataCopier with the given \p translator.
    CellDataCopier(CellRefTranslator<T, U> * translator) :
        translator_(translator)
    {
    }

    /// Copy the given cell data \p 'from' into the given cell data \p 'to',
    /// translating its references using the \p translator given to the
    /// constructor of this CellDataCopier. Assumes that \p 'from' and
    /// \p 'to' have the same data type, and aborts otherwise.
    void copy(const CellData<T> & from, CellData<U> & to)
    {
        assert(from.type() == to.type());
        CellDataCopierVisitor<T, U> visitor(translator_, &to);
        from.accept(visitor);
    }
};

} // end namespace OpenVac

#endif // OPENVAC_CELLDATACOPIER_H

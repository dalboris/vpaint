// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_VAC_H
#define OPENVAC_VAC_H

#include <OpenVAC/Geometry/GManager.h>

#include <memory>

namespace OpenVAC
{

class VAC
{
public:
    // Construct a VAC. The VAC takes ownership of gManager.
    VAC(GManager * gManager = nullptr);

private:
    // Geometric data
    std::unique_ptr<GManager> gManager_;
};

}

#endif // OPENVAC_ VAC_H

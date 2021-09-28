// Copyright (C) 2012-2019 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SVGIMPORTPARAMS_H
#define SVGIMPORTPARAMS_H

#include <QObject>
#include <QString>
#include "vpaint_global.h"

enum class SvgImportVertexMode {
    All,
    //Corners,
    Endpoints
};

constexpr auto defaultSvgImportVertexMode = SvgImportVertexMode::All;

inline SvgImportVertexMode toSvgImportVertexMode(const QString& s) {
    if (s == "all")            return SvgImportVertexMode::All;
    //else if (s == "corners")   return VertexMode::Corners;
    else if (s == "endpoints") return SvgImportVertexMode::Endpoints;
    else return defaultSvgImportVertexMode;
}

inline QString toString(SvgImportVertexMode m) {
    switch (m) {
    case SvgImportVertexMode::All:       return "all";
    //case SvgImportVertexMode::Corners:   return "corners";
    case SvgImportVertexMode::Endpoints: return "endpoints";
    }
    return QString();
}

struct Q_VPAINT_EXPORT SvgImportParams {
    SvgImportVertexMode vertexMode;
};

#endif // SVGIMPORTPARAMS_H

// Copyright (C) 2012-2023 The VPaint Developers.
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

#include "ExportSettings.h"

namespace {

std::vector<ExportFileTypeInfo> createFileTypes() {
    std::vector<ExportFileTypeInfo> types;
    using C = ExportFileTypeCategory;
    types.emplace_back("svg", "SVG Image", C::VectorImage);
    types.emplace_back("png", "PNG Image", C::RasterImage);
    return types;
}

} // namespace

const std::vector<ExportFileTypeInfo>& exportFileTypes() {
    static std::vector<ExportFileTypeInfo> types = createFileTypes();
    return types;
}

RasterExportSettings::RasterExportSettings()
{
}

VectorExportSettings::VectorExportSettings()
{
}

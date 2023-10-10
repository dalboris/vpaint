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

#ifndef EXPORT_SETTINGS_H
#define EXPORT_SETTINGS_H

#include <string>
#include <vector>

/// \class ExportFileTypeInfo
/// \brief Specifies broad categories of file types.
///
enum class ExportFileTypeCategory {
    RasterImage,
    VectorImage
    // RasterVideo
    // VectorVideo
};

/// \class ExportFileTypeInfo
/// \brief Specifies whether to export a single image or an image sequence.
///
enum class FrameRangeType {
    SingleImage,
    ImageSequenceAll
    // ImageSequenceCustomRange + additional getters startFrame()/endFrame()
};

/// \class ExportFileTypeInfo
/// \brief Specifies meta-information about a given file type.
///
///
class ExportFileTypeInfo
{
public:
    /// Creates an `ExportFileTypeInfo`, with the given `extension` (without
    /// the leading dot) and the given `name`.
    ///
    ExportFileTypeInfo(
        const std::string& extension,
        const std::string& name,
        ExportFileTypeCategory category)

        : extension_(extension)
        , name_(name)
        , category_(category) {
    }

    /// Returns the extension of this file type (without the leading dot).
    ///
    /// Example: `svg`.
    ///
    /// Note that two different `ExportFileTypeInfo` may have the same
    /// extension. For example, exporting as an SVG image sequence or as an SVG
    /// animation (SMIL) are two different export options with different
    /// `category()`: the first is `VectorImage`, while the second is
    /// `VectorVideo`.
    ///
    const std::string& extension() const {
        return extension_;
    }

    /// Returns the name of this file type.
    ///
    /// Example: `SVG Image`.
    ///
    const std::string& name() const {
        return name_;
    }

    /// Returns the category of this file type.
    ///
    ExportFileTypeCategory category() const {
        return category_;
    }

private:
    std::string extension_;
    std::string name_;
    ExportFileTypeCategory category_;
};

/// Returns a list of registered file types.
///
const std::vector<ExportFileTypeInfo>& exportFileTypes();

class RasterExportSettings
{
public:
    RasterExportSettings();

    // Image size

    int pngWidth() const {
        return pngWidth_;
    }

    void setPngWidth(int w) {
        pngWidth_ = w;
    }

    int pngHeight() const {
        return pngHeight_;
    }

    void setPngHeight(int h) {
        pngHeight_ = h;
    }

    // Whether to render using view settings
    // Note: might be useful for RasterExportSettings too.

    bool useViewSettings() const {
        return useViewSettings_;
    }

    void setUseViewSettings(bool value) {
        useViewSettings_ = value;
    }


    // Motion blur
    bool motionBlur() const {
        return motionBlur_;
    }

    void setMotionBlur(bool value) {
        motionBlur_ = value;
    }

    int motionBlurNumSamples() const {
        return motionBlurNumSamples_;
    }

    void setMotionBlurNumSamples(int value) {
        motionBlurNumSamples_ = value;
    }

private:
    int pngWidth_ = 0;
    int pngHeight_ = 0;

    bool useViewSettings_ = false;

    bool motionBlur_ = false;
    int motionBlurNumSamples_ = 1;
};

class VectorExportSettings
{
public:
    VectorExportSettings();

    bool backgroundAsRect() const {
        return backgroundAsRect_;
    }

    void setBackgroundAsRect(bool value) {
        backgroundAsRect_ = value;
    }

    bool fillVariableWidthStrokes() const {
        return fillVariableWidthStrokes_;
    }

    void setFillVariableWidthStrokes(bool value) {
        fillVariableWidthStrokes_ = value;
    }

private:
    bool backgroundAsRect_ = false;
    bool fillVariableWidthStrokes_ = false;
};

#endif

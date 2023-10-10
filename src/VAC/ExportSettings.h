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

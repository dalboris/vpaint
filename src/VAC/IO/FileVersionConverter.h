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

#ifndef FILE_VERSION_CONVERTER_H
#define FILE_VERSION_CONVERTER_H

#include <QString>
#include "VAC/vpaint_global.h"

class QWidget;
class XmlStreamReader;
class XmlStreamWriter;

class Q_VPAINT_EXPORT FileVersionConverter
{
public:
    FileVersionConverter(const QString & filePath);

    // Get version info about file
    QString fileVersion() const;
    int fileMajor() const;
    int fileMinor() const;

    // Converts file to new version if required.
    // If popupParent is non null, and conversion is required, then
    // it asks the user whether to convert or abort the operation
    //
    // Returns true if no need to convert, or successfully converted.
    // Returns false if conversion failed, or aborted by user
    bool convertToVersion(
            const QString & targetVersion,
            QWidget * popupParent = 0);

private:
    QString filePath_;
    QString fileVersion_;
    int fileMajor_;
    int fileMinor_;

    void readVersion_();
};

#endif

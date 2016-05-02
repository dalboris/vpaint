// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef FILE_VERSION_CONVERTER_H
#define FILE_VERSION_CONVERTER_H

#include <QString>

class QWidget;
class XmlStreamReader;
class XmlStreamWriter;

class FileVersionConverter
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

// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "FileVersionConverter.h"

#include "XmlStreamReader.h"

#include <QFile>

FileVersionConverter::FileVersionConverter(const QString & filePath) :
    filePath_(filePath),
    version_(""),
    majorVersion_(0),
    minorVersion_(0)
{
    readVersion_();
}

QString FileVersionConverter::version() const
{
    return version_;
}

int FileVersionConverter::majorVersion() const
{
    return majorVersion_;
}

int FileVersionConverter::minorVersion() const
{
    return minorVersion_;
}

void FileVersionConverter::readVersion_()
{
    // Open file
    QFile file(filePath_);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return;

    // Parse XML to get version
    XmlStreamReader xml(&file);
    if (xml.readNextStartElement() &&
        xml.name() == "vec" &&
        xml.attributes().hasAttribute("version"))
    {
        // Get version as string
        version_ = xml.attributes().value("version").toString();

        // Split string version at dots and spaces
        QStringList list = version_.split(QRegExp("\\.| "));

        // Extract major and minor integers
        if (list.size() >= 2)
        {
            majorVersion_ = list[0].toInt();
            minorVersion_ = list[1].toInt();
        }
    }

    // Close file
    file.close();
}

bool FileVersionConverter::convertToVersion(
        const QString & version,
        QWidget * popupParent)
{
    return true;
}

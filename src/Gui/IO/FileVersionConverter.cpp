// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "FileVersionConverter.h"

#include "XmlStreamReader.h"
#include "XmlStreamWriter.h"

#include "XmlStreamConverters/XmlStreamConverter_1_0_to_1_6.h"

#include <QPair>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

FileVersionConverter::FileVersionConverter(const QString & filePath) :
    filePath_(filePath),
    fileVersion_(""),
    fileMajor_(0),
    fileMinor_(0)
{
    readVersion_();
}

QString FileVersionConverter::fileVersion() const
{
    return fileVersion_;
}

int FileVersionConverter::fileMajor() const
{
    return fileMajor_;
}

int FileVersionConverter::fileMinor() const
{
    return fileMinor_;
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
        fileVersion_ = xml.attributes().value("version").toString();

        // Split string version at dots and spaces
        QStringList list = fileVersion_.split(QRegExp("\\.| "));

        // Extract major and minor integers
        if (list.size() >= 2)
        {
            fileMajor_ = list[0].toInt();
            fileMinor_ = list[1].toInt();
        }
    }

    // Close file
    file.close();
}

bool FileVersionConverter::convertToVersion(
        const QString & targetVersion,
        QWidget * popupParent)
{
    // Get target minor and major
    int targetMajor = 0;
    int targetMinor = 0;
    QStringList list = targetVersion.split(QRegExp("\\.| "));
    if (list.size() >= 2)
    {
        targetMajor = list[0].toInt();
        targetMinor = list[1].toInt();
    }
    else
    {
        return false;
    }

    // Store as pair for easy comparison
    QPair<int,int> fromVersion = qMakePair(fileMajor(), fileMinor());
    QPair<int,int> toVersion = qMakePair(targetMajor, targetMinor);

    // Nothing to do if the versions match
    if (fromVersion == toVersion)
    {
        return true;
    }

    // Fail if trying to convert from a newer version
    // For now, we only support opening of old files with new versions of VPaint, not
    // the other way around
    else if (fromVersion > toVersion)
    {
        QMessageBox::warning(
                    popupParent, QObject::tr("VPaint too old to read this file"),
                    QObject::tr("This file was created with a newer version of VPaint and I can't open it! "
                       "Please download the latest version of VPaint at http://www.vpaint.org"));
        return false;
    }

    // Convert to new format if fromVersion == 1.0
    else if (fromVersion == qMakePair(1,0))
    {
        // Get backup path
        QFileInfo fileInfo(filePath_);
        QString fileName = fileInfo.fileName();
        QString backupFileName =
                fileInfo.completeBaseName() +
                ".old." +
                fileInfo.suffix();
        QString backupPath =
                fileInfo.path() +
                "/" +
                backupFileName;

        // Show popup to warn that a conversion will happen
        QMessageBox::warning(
                    popupParent, QObject::tr("File format conversion required"),
                    QObject::tr("The file %1 comes from an older version of VPaint and "
                                "must be converted to a new *.vec version. Do you want "
                                "to proceed? The old file will still be available at %2")
                    .arg(fileName, backupFileName));

        // Create backup
        QDir dir = fileInfo.dir();
        if (!dir.rename(fileName, backupFileName))
        {
            QMessageBox::warning(
                        popupParent, QObject::tr("Conversion failed"),
                        QObject::tr("Oops... I couldn't copy the file to %1, so I aborted "
                                    "the operation. Maybe I don't have write access to this "
                                    "directory? Or the file already exist?").arg(backupFileName));
        }

        // Open file for reading
        QFile inFile(backupPath);
        if (!inFile.open(QFile::ReadOnly | QFile::Text))
        {
            QMessageBox::warning(
                        popupParent, QObject::tr("Conversion failed"),
                        QObject::tr("Oops... I couldn't read %1 to perform the conversion, "
                                    "so I aborted the operation").arg(backupFileName));
        }

        // Open file for writing
        QFile outFile(filePath_);
        if (!outFile.open(QFile::WriteOnly | QFile::Text))
        {
            QMessageBox::warning(
                        popupParent, QObject::tr("Conversion failed"),
                        QObject::tr("Oops... I couldn't open %1 for writing the converted "
                                    "file, so I aborted the operation. Maybe I don't have write "
                                    "access to that file?").arg(filePath_));
        }

        // Perform the conversion
        XmlStreamReader inXml(&inFile);
        XmlStreamWriter outXml(&outFile);
        XmlStreamConverter_1_0_to_1_6(inXml, outXml).traverse();

        // Close files
        inFile.close();
        outFile.close();

        // Success!
        return true;
    }

    // Nothing to do otherwise (e.g., fromVersion = 1.6 and toVersion == 1.7
    // without a breaking change between 1.6 and 1.7)
    else
    {
        return true;
    }
}

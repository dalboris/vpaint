#include "DirUtils.h"

#include <QMessageBox>

bool DirUtils::cd(QDir & dir, const QString & dirPath)
{
    DirUtils::mkdir(dir, dirPath);

    QStringList names = dirPath.split('/');
    foreach (QString name, names)
    {
        if (!dir.cd(name))
        {
            QMessageBox::critical(
                        0, "Error moving to directory",
                        QString("Failed to move to directory %1")
                        .arg(dir.absoluteFilePath(name)));
            return false;
        }
    }
    return true;
}

bool DirUtils::mkdir(const QDir & dir_, const QString & dirPath)
{
    QStringList names = dirPath.split('/');
    QDir dir = dir_;
    foreach (QString name, names)
    {
        // Create directory if it does not exist yet
        if (!dir.exists(name))
        {
            if (!dir.mkdir(name))
            {
                QMessageBox::critical(
                        0, "Error creating directory",
                        QString("Failed to create directory %1")
                            .arg(dir.absoluteFilePath(name)));
                return false;
            }
        }

        // Move to directory
        if (!dir.cd(name))
        {
            QMessageBox::critical(
                        0, "Error moving to directory",
                        QString("Failed to move to directory %1")
                        .arg(dir.absoluteFilePath(name)));
            return false;
        }
    }
    return true;
}

bool DirUtils::isDir(const QDir & dir_, const QString & dirPath)
{
    QStringList names = dirPath.split('/');
    QDir dir = dir_;
    foreach (QString name, names)
    {
        // Check if the directory exists
        if (!dir.exists(name))
        {
            return false;
        }

        // Check that it can be traversed
        if (!dir.cd(name))
        {
            return false;
        }
    }
    return true;
}

QDir DirUtils::rootDir()
{
    // Note: num of cdUp() depends on win32/unix
    QDir res = QDir(QMAKE_PWD);
    while (res.dirName() != "tests")
        res.cdUp();
    res.cdUp();
    return res;
}

QDir DirUtils::rootOutDir()
{
    // Note: num of cdUp() depends on win32/unix
    QDir res = QDir(QMAKE_OUT_PWD);
    while (res.dirName() != "tests")
        res.cdUp();
    res.cdUp();
    return res;
}

QDir DirUtils::dir(const QString & relPath)
{
    QDir res = rootDir();
    cd(res, relPath);
    return res;
}

QDir DirUtils::outDir(const QString & relPath)
{
    QDir res = rootOutDir();
    cd(res, relPath);
    return res;
}

#ifndef DIRUTILS_H
#define DIRUTILS_H

#include <QDir>
#include <QString>

/// \class DirUtils
/// \brief Convenient class to perform operations on directories
///
class DirUtils
{
public:
    /// Performs cd operations on the dir to get to \p dirPath. Silently
    /// performs an mkdir operation for each directory in \p dirPath that does
    /// not already exists. The given \p dirPath is relative to \p dir.
    ///
    /// Returns true if dir was successfully moved to \p dirPath.
    ///
    /// Returns false and logs an error if a directory in \p dirPath did not
    /// exist and mkdir failed; or if a directory in \p dirPath existed (or was
    /// successfully created) but traversing it failed (e.g., due to
    /// permissions).
    ///
    static bool cd(QDir & dir, const QString & dirPath);

    /// Performs an mkdir operation for each directory in \p dirPath that does
    /// not already exists. The given \p dirPath is relative to \p dir.
    ///
    /// Returns true if each directory in \p dirPath was successfully created
    /// or already existed.
    ///
    /// Returns false and logs an error if a directory in \p dirPath did not
    /// exist and mkdir failed; or if a directory in \p dirPath existed (or was
    /// successfully created) but traversing it failed (for instance, due to
    /// permissions).
    ///
    static bool mkdir(const QDir & dir, const QString & dirPath);

    /// Returns true if the given \p dirPath exists, is a directory, and
    /// can be traversed.
    ///
    static bool isDir(const QDir & dir, const QString & dirPath);

    /// Returns the distribution's root directory. Example:
    ///     /home/boris/Documents/QtProjectTemplate
    ///
    static QDir rootDir();

    /// Returns the distribution's root out directory. Example:
    ///     /home/boris/Documents/build-QtProjectTemplate-Desktop_Qt_5_5_1_GCC_64bit-Release
    ///
    static QDir rootOutDir();

    /// Returns the directory given by \p relPath, relative to rootDir()
    ///
    static QDir dir(const QString & relPath);

    /// Returns the out directory given by \p relPath, relative to outRootDir()
    ///
    static QDir outDir(const QString & relPath);
};

#endif // DIRUTILS_H

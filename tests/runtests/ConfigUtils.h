#ifndef CONFIGUTILS_H
#define CONFIGUTILS_H

#include <QString>
#include <QStringList>

/// \class ConfigUtils
/// \brief Convenient class to get basic build configuration
///
class ConfigUtils
{
public:
    static bool isRelease();
    static bool isDebug();
    static QString releaseOrDebug();

    static bool isWin32();
    static bool isUnix();

    static QStringList config;
};

#endif // CONFIGUTILS_H

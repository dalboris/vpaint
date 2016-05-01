#include "ConfigUtils.h"

QStringList ConfigUtils::config = QString(QMAKE_CONFIG).split(' ');

bool ConfigUtils::isRelease()
{
    bool res = false;
    foreach (QString s, config)
    {
        if (s == "release")
            res = true;
        else if (s == "debug")
            res = false;
    }
    return res;
}

bool ConfigUtils::isDebug()
{
    bool res = false;
    foreach (QString s, config)
    {
        if (s == "release")
            res = false;
        else if (s == "debug")
            res = true;
    }
    return res;
}

QString ConfigUtils::releaseOrDebug()
{
    return isRelease() ? "release" : "debug";
}

bool ConfigUtils::isWin32()
{
    return config.contains("win32");
}

bool ConfigUtils::isUnix()
{
    return config.contains("unix");
}

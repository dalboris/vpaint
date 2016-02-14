// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Version.h"

#include <QStringList>

Version::Version(const QString & str)
{
    major_ = minor_ = patch_ = 0;

    if(str.trimmed().isEmpty()) return;

    QStringList split = str.split(".");
    if(split.size() > 0)
    {
        major_ = split.at(0).toShort();
    }
    if(split.size() > 1)
    {
        minor_ = split.at(1).toShort();
    }
    if(split.size() > 2)
    {
        patch_ = split.at(2).toShort();
    }
}

short Version::getMajor() const
{
    return major_;
}

void Version::setMajor(short val)
{
    major_ = val;
}

short Version::getMinor() const
{
    return minor_;
}

void Version::setMinor(short val)
{
    minor_ = val;
}

short Version::getPatch() const
{
    return patch_;
}

void Version::setPatch(short val)
{
    patch_ = val;
}

bool Version::operator< (const Version& other) const
{
    if(getMajor() > other.getMajor()) return false;
    if(getMajor() < other.getMajor()) return true;

    if(getMinor() > other.getMinor()) return false;
    if(getMinor() < other.getMinor()) return true;

    if(getPatch() > other.getPatch()) return false;
    if(getPatch() < other.getPatch()) return true;

    return false;
}

QString Version::toString(bool ignorePatch)
{
    if (ignorePatch || getPatch() == 0)
    {
        return QString("%1.%2").arg(getMajor()).arg(getMinor());
    }
    else
    {
        return QString("%1.%2.%3").arg(getMajor()).arg(getMinor()).arg(getPatch());
    }
}

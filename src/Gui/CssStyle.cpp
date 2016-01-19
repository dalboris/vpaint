// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "CssStyle.h"

#include <QStringList>

CssStyle::CssStyle()
{
}

int CssStyle::size() const
{
    return style_.size();
}

bool CssStyle::contains(const QString & name) const
{
    return style_.contains(name);
}

QString CssStyle::get(const QString & name) const
{
    if(contains(name))
        return style_[name];
    else
        return QString();
}

void CssStyle::clear()
{
    style_.clear();
}

void CssStyle::set(const QString & name, const QString & value)
{
    style_[name] = value;
}

void CssStyle::remove(const QString & name)
{
    style_.remove(name);
}

QString CssStyle::toString() const
{
    QString res;

    bool first = true;
    for(auto it = style_.cbegin(); it != style_.cend(); ++it)
    {
        if(first)
            first = false;
        else
            res += ";";

        res += it.key() + ":" + it.value();
    }

    return res;
}

void CssStyle::fromString(const QString & c)
{
    clear();

    QStringList pairs = c.split(";", QString::SkipEmptyParts);
    foreach(QString pair, pairs)
    {
        QStringList d = pair.split(":", QString::SkipEmptyParts);
        if(d.size() == 2)
            set(d[0].simplified(), d[1].simplified());
    }
}

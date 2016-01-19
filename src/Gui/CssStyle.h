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

#ifndef CSSSTYLE_H
#define CSSSTYLE_H

#include <QMap>
#include <QString>

class CssStyle
{
public:
    CssStyle();

    // General info
    int size() const; // num of style attributes

    // Get
    bool contains(const QString & name) const;
    QString get(const QString & name) const;

    // Set
    void clear();
    void set(const QString & name, const QString & value);
    void remove(const QString & name);

    // String input/output as "color: rgba(r,g,b,a) ; stroke-width:10;"
    //   1. trailing semicolon optional
    //   2. names and values cannot contain ':' or ';'
    //   3. names cannot contain whitespaces
    QString toString() const;
    void fromString(const QString & c);

private:
    QMap<QString,QString> style_;
};

#endif // CSSSTYLE_H

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

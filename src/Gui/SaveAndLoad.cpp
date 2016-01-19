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

#include "SaveAndLoad.h"
#include <QTextStream>

int Save::indent_ = 0;


Field::Field() :
    string_()
{
}

Field::Field(const QString & string) :
    string_(string)
{
}


QTextStream & operator<<(QTextStream & str, const Field & field)
{
    str << (Save::newField(field.string_));
    return str;
}

QTextStream & operator>>(QTextStream & str, Field & field)
{
    field = Read::field(str);
    return str;
}

    
QString Save::indent(int n)
{
    QString res;
    for(int i=0; i<n; i++) res += " ";
    return res;
}

void Save::resetIndent()
{
    indent_ = 0;
}

void Save::incrIndent()
{
    indent_ += 4;
}

void Save::decrIndent()
{
    indent_ -= 4;
}

QString Save::indent()
{
    return indent(indent_);
}


QString Save::newField(const QString & fieldName)
{
    return "\n" + indent() + fieldName + " : ";
}

QString Save::openCurlyBrackets()
{
    QString res = "\n" + indent() + "{";
    incrIndent();
    return res;
}

QString Save::closeCurlyBrackets()
{
    decrIndent();
    return "\n" + indent() + "}";
}

QString Read::field(QTextStream & in)
{
    QString res, colon;
    in >> res >> colon;
    return res;
}
QString Read::string(QTextStream & in)
{
    QString res;
    in >> res;
    return res;
}

#include <QtDebug>

void Read::skipBracket(QTextStream & in)
{
    QString skip;
    in >> skip;
    //qDebug() << skip;
}

QString Read::readBracketedBlock(QTextStream & in)
{
    QString res;

    // Read first opening bracket, ignore everything which is before
    unsigned int openedBracket = 0;
    char c;
    while(!openedBracket)
    {
        in >> c;
        if(c == '[' || c == '(' || c == '{')
        {
            openedBracket++;
            res.append(c);
        }
    }

    // Read until match found
    while(openedBracket)
    {
        in >> c;
        res.append(c);
        if(c == '[' || c == '(' || c == '{')
            openedBracket++;
        else if(c == ']' || c == ')' || c == '}')
            openedBracket--;
    }

    return res;
}

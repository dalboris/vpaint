// Copyright (C) 2012-2019 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SAVE_AND_LOAD_H
#define SAVE_AND_LOAD_H

#include <QString>
#include <QList>
#include <QTextStream>
#include "VAC/vpaint_global.h"

class Q_VPAINT_EXPORT Field
{
public:
    Field();
    Field(const QString & string);

    QString string() {return string_;}
    
    friend QTextStream & operator<<(QTextStream &, const Field & field);
    friend QTextStream & operator>>(QTextStream &, Field & field);
    
private:
    QString string_;
};

class Q_VPAINT_EXPORT Save
{
public:
    static QString indent(int n);
    
    static void resetIndent();    
    static void incrIndent();    
    static void decrIndent();
    static QString indent();
        
    static QString newField(const QString & fieldName);
    static QString openCurlyBrackets();
    static QString closeCurlyBrackets();
        
private:
    static int indent_;
};

class Q_VPAINT_EXPORT Read
{
public:
    static QString string(QTextStream & in);
    static QString field(QTextStream & in);
    static void skipBracket(QTextStream & in);
    static QString readBracketedBlock(QTextStream & in);
};




template<class T>
QTextStream & operator<<(QTextStream & out, const QList<T> & list)
{
    out << "[";
    for(int i=0; i<list.size(); ++i)
    {
        if(i!=0) out << " ,";
        out << " " << list[i];
    }
    out << " ]";

    return out;
}
    
template<class T>
QTextStream & operator>>(QTextStream & in, QList<T> & list)
{
    QString listAsString = Read::readBracketedBlock(in);

    bool isEmpty = false;
    if(listAsString == "[]" || listAsString == "[ ]")
        isEmpty = true;

    // if not empty
    if(!isEmpty)
    {
        QTextStream newIn(&listAsString);
        QString delimiter;
        newIn >> delimiter; // read "["
        delimiter = ",";
        while(delimiter == ",")
        {
            T t;
            newIn >> t;
            list << t;
            newIn >> delimiter;
        }
    }

    return in;
}

template<class T, class U>
QTextStream & operator<<(QTextStream & out, const QPair<T,U> & pair)
{
    out << "( " << pair.first << " , " << pair.second << " )";
    return out;
}

template<class T, class U>
QTextStream & operator>>(QTextStream & in, QPair<T,U> & pair)
{
    QString s;
    in >> s >> pair.first >> s >> pair.second >> s;
    return in;
}

#endif

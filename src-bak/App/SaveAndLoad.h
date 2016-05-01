// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef SAVE_AND_LOAD_H
#define SAVE_AND_LOAD_H

#include <QString>
#include <QList>
#include <QTextStream>

class Field
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

class Save
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

class Read
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

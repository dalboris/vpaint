// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2016 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "BackgroundUrlValidator.h"

BackgroundUrlValidator::BackgroundUrlValidator(QObject * parent) :
    QValidator(parent)
{
}

QValidator::State BackgroundUrlValidator::validateUrl(QString & input, int & /*pos*/)
{
    // Make sure that there are no more than one wildcard, and
    // no slash after wildcards.
    //
    // XXX We should in addition check that it's a valid url
    // if we remove the (potential) wildcard

    int wildcardCount = 0;
    for (int i=0; i<input.size(); ++i)
    {
        if (input[i] == '*')
        {
            if(wildcardCount == 0)
                wildcardCount++;
            else
                return QValidator::Invalid;
        }
        else if (input[i] == '/' && wildcardCount > 0)
        {
            return QValidator::Invalid;
        }
    }

    return QValidator::Acceptable;
}

void BackgroundUrlValidator::fixupUrl(QString & input)
{
    // Get last index of '*' or '/' (return -1 if not found)
    int j = input.lastIndexOf('*');
    int k = input.lastIndexOf('/');

    // Remove '*' if followed by '/'
    if (k > j)
        j = -1;

    // Only keep wildcard at index j
    QString res;
    for (int i=0; i<input.size(); ++i)
        if (input[i] != '*' || i==j)
            res.append(input[i]);

    // Set result
    input = res;
}

QValidator::State BackgroundUrlValidator::validate(QString & input, int & pos) const
{
    return validateUrl(input, pos);
}

void BackgroundUrlValidator::fixup(QString & input) const
{
    fixupUrl(input);
}

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

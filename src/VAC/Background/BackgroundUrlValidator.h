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

#ifndef BACKGROUND_URL_VALIDATOR_H
#define BACKGROUND_URL_VALIDATOR_H

#include <QValidator>
#include "vpaint_global.h"

class Q_VPAINT_EXPORT BackgroundUrlValidator: public QValidator
{
public:
    BackgroundUrlValidator(QObject * parent = 0);

    // Reimplementations
    QValidator::State validate(QString & input, int & pos) const;
    void fixup(QString & input) const;

    // Do the same as above, but as static functions
    static QValidator::State validateUrl(QString & input, int & pos);
    static void fixupUrl(QString & input);

};

#endif // BACKGROUND_URL_VALIDATOR_H

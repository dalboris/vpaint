// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2016 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef BACKGROUND_URL_VALIDATOR_H
#define BACKGROUND_URL_VALIDATOR_H

#include <QValidator>

class BackgroundUrlValidator: public QValidator
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

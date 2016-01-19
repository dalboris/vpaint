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

#include "GLWidget_Settings.h"

#include <QGridLayout>

GLWidget_Settings::GLWidget_Settings()
{
    backgroundColor_r_ = new QDoubleSpinBox();
    backgroundColor_r_->setMinimum(0);
    backgroundColor_r_->setMaximum(1);
    backgroundColor_r_->setValue(1);
    connect(backgroundColor_r_, SIGNAL(valueChanged(double)),
          this, SIGNAL(changed()));

    backgroundColor_g_ = new QDoubleSpinBox();
    backgroundColor_g_->setMinimum(0);
    backgroundColor_g_->setMaximum(1);
    backgroundColor_g_->setValue(1);
    connect(backgroundColor_g_, SIGNAL(valueChanged(double)),
          this, SIGNAL(changed()));

    backgroundColor_b_ = new QDoubleSpinBox();
    backgroundColor_b_->setMinimum(0);
    backgroundColor_b_->setMaximum(1);
    backgroundColor_b_->setValue(1);
    connect(backgroundColor_b_, SIGNAL(valueChanged(double)),
          this, SIGNAL(changed()));

    drawGrid_ = new QCheckBox();
    drawGrid_->setChecked(true);
    connect(drawGrid_, SIGNAL(toggled(bool)),
          this, SIGNAL(changed()));
    
    QGridLayout *layout = new QGridLayout();
    layout->addWidget(backgroundColor_r_, 0, 0);
    layout->addWidget(backgroundColor_g_, 1, 0);
    layout->addWidget(backgroundColor_b_, 2, 0);
    layout->addWidget(drawGrid_, 3, 0);

    setLayout(layout);
}

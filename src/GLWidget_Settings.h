// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef GLWIDGET_SETTINGS_H
#define GLWIDGET_SETTINGS_H

#include <QWidget>
#include <QCheckBox>
#include <QDoubleSpinBox>

class GLWidget_Settings: public QWidget
{
    Q_OBJECT
    
public:
    GLWidget_Settings();

    // Draw grid
    void setDrawGrid(bool b)
        {drawGrid_->setChecked(b);}
    bool drawGrid() const
        {return drawGrid_->isChecked();}
    
    // Background color
    void setBackgroundColor(double r, double g, double b)
        {backgroundColor_r_->setValue(r);
         backgroundColor_g_->setValue(g);
         backgroundColor_b_->setValue(b);}
    double backgroundColor_r() const
        {return backgroundColor_r_->value();}
    double backgroundColor_g() const
        {return backgroundColor_g_->value();}
    double backgroundColor_b() const
        {return backgroundColor_b_->value();}
    

signals:
    void changed();

private:
    QCheckBox *drawGrid_;
    
    QDoubleSpinBox *backgroundColor_r_;
    QDoubleSpinBox *backgroundColor_g_;
    QDoubleSpinBox *backgroundColor_b_;    
};

    
#endif

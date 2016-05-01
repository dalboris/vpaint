// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef EXPORTPNGDIALOG_H
#define EXPORTPNGDIALOG_H

#include <QDialog>

class QCheckBox;
class QSpinBox;
class QDoubleSpinBox;
class SceneOld;

class ExportPngDialog: public QDialog
{
    Q_OBJECT

public:
    ExportPngDialog(SceneOld * scene);

    // Reimplements from QDialog
    void setVisible(bool visible);

    // Access linked scene
    SceneOld * scene() const;

    // Access canvas settings
    double left() const;
    double top() const;
    double width() const;
    double height() const;

    // Access png settings
    int pngWidth() const;
    int pngHeight() const;
    bool preserveAspectRatio() const;
    bool exportSequence() const;

public slots:
    // Reimplements from QDialog
    void accept();
    void reject();

    // Backend <-> Frontend conversions
    void updateDialogFromScene();
    void updateSceneFromDialog();

private slots:
    void processCanvasSizeChanged_();
    void processPngWidthChanged_(int w);
    void processPngHeightChanged_(int h);
    void processPreserveAspectRatioChanged_(bool b);

private:
    SceneOld * scene_;

    QDoubleSpinBox * topSpinBox_;
    QDoubleSpinBox * leftSpinBox_;
    QDoubleSpinBox * widthSpinBox_;
    QDoubleSpinBox * heightSpinBox_;

    QSpinBox * pngWidthSpinBox_;
    QSpinBox * pngHeightSpinBox_;
    QCheckBox * preserveAspectRatioCheckBox_;
    QCheckBox * exportSequenceCheckBox_;

    double oldTop_;
    double oldLeft_;
    double oldWidth_;
    double oldHeight_;

    bool ignoreSceneChanged_;
    bool ignoreWidthHeightChanged_;

    void backupCurrentCanvasSize_();
    void enforcePngAspectRatio_();
    void setPngWidthForHeight_();
    void setPngHeightForWidth_();
};


#endif // EXPORTPNGDIALOG_H

// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef EXPORTPNGDIALOG_H
#define EXPORTPNGDIALOG_H

#include <QDialog>

class QCheckBox;
class QSpinBox;
class QDoubleSpinBox;
class Scene;

class ExportPngDialog: public QDialog
{
    Q_OBJECT

public:
    ExportPngDialog(Scene * scene);

    // Reimplements from QDialog
    void setVisible(bool visible);

    // Access linked scene
    Scene * scene() const;

    // Access canvas settings
    double left() const;
    double top() const;
    double width() const;
    double height() const;

    // Access png settings
    int pngWidth() const;
    int pngHeight() const;
    bool preserveAspectRatio() const;
    bool transparentBackground() const;
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
    Scene * scene_;

    QDoubleSpinBox * topSpinBox_;
    QDoubleSpinBox * leftSpinBox_;
    QDoubleSpinBox * widthSpinBox_;
    QDoubleSpinBox * heightSpinBox_;

    QSpinBox * pngWidthSpinBox_;
    QSpinBox * pngHeightSpinBox_;
    QCheckBox * preserveAspectRatioCheckBox_;
    QCheckBox * transparentBackgroundCheckBox_;
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

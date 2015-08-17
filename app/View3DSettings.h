// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef VIEW3DSETTINGS_H
#define VIEW3DSETTINGS_H

#include "TimeDef.h"
#include <QWidget>

class View3DSettingsWidget;
class View3DSettings
{
public:
    View3DSettings();
    ~View3DSettings();

    // Scaling
    double spaceScale() const; // xyOpenGL = xyScene * spaceScale
    void setSpaceScale(double newValue);
    double timeScale() const; // zOpenGL = t * timeScale
    void setTimeScale(double newValue);
    bool isTimeHorizontal() const;
    void setIsTimeHorizontal(bool newValue);
    bool freezeSpaceRect() const;
    void setFreezeSpaceRect(bool newValue);
    bool cameraFollowActiveTime() const;
    void setCameraFollowActiveTime(bool newValue);



    bool drawGrid() const;
    void setDrawGrid(bool newValue);
    bool drawTimePlane() const;
    void setDrawTimePlane(bool newValue);
    bool drawCurrentFrame() const;
    void setDrawCurrentFrame(bool newValue);
    bool drawAllFrames() const;
    void setDrawAllFrames(bool newValue);
    bool drawFramesAsTopology() const;
    void setDrawFramesAsTopology(bool newValue);
    bool drawCurrentFrameAsTopology() const;
    void setDrawCurrentFrameAsTopology(bool newValue);
    bool drawTopologyFaces() const;
    void setDrawTopologyFaces(bool newValue);
    bool drawKeyCells() const;
    void setDrawKeyCells(bool newValue);
    bool drawInbetweenCells() const;
    void setDrawInbetweenCells(bool newValue);
    bool drawKeyVerticesAsDots() const;
    void setDrawKeyVerticesAsDots(bool newValue);
    bool clipToSpaceTimeWindow() const;
    void setClipToSpaceTimeWindow(bool newValue);


    int vertexTopologySize() const;
    void setVertexTopologySize(int newValue);

    int edgeTopologyWidth() const;
    void setEdgeTopologyWidth(int newValue);


    // 3D surface drawing
    double opacity() const;
    void setOpacity(double newValue);
    bool drawAsMesh() const;
    void setDrawAsMesh(bool newValue);
    int k1() const;
    void setK1(int newValue);
    int k2() const;
    void setK2(int newValue);






    double xFromXScene(double xScene) const;
    double yFromYScene(double yScene) const;

    double zFromT(double time) const;
    double zFromT(int time) const;
    double zFromT(Time time) const;

    // Scene settings
    double xSceneMin() const;
    double xSceneMax() const;
    double ySceneMin() const;
    double ySceneMax() const;

    void setXSceneMin(double value);
    void setXSceneMax(double value);
    void setYSceneMin(double value);
    void setYSceneMax(double value);


private:
    // Display settings
    double spaceScale_;
    double timeScale_;
    bool isTimeHorizontal_;
    bool freezeSpaceRect_;
    bool cameraFollowActiveTime_;

    bool drawGrid_;
    bool drawTimePlane_;
    bool drawCurrentFrame_;
    bool drawAllFrames_;
    bool drawFramesAsTopology_;
    bool drawCurrentFrameAsTopology_;
    bool drawTopologyFaces_;
    bool drawKeyCells_;
    bool drawInbetweenCells_;
    bool drawKeyVerticesAsDots_;
    bool clipToSpaceTimeWindow_;

    int vertexTopologySize_;
    int edgeTopologyWidth_;

    // 3D
    double opacity_;
    bool drawAsMesh_;
    int k1_;
    int k2_;

    // Scene settings
    double xSceneMin_, xSceneMax_, ySceneMin_, ySceneMax_;
};

#include <QFormLayout>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>

class View3DSettingsWidget: public QWidget
{
    Q_OBJECT

public:
    View3DSettingsWidget(View3DSettings & viewSettings);
    ~View3DSettingsWidget();

signals:
    void changed();
    void closed();

protected slots:
    void closeEvent(QCloseEvent * event);

private slots:
    void updateWidgetFromSettings();
    void updateSettingsFromWidget();

private:
    View3DSettings & viewSettings_;

    QDoubleSpinBox * spaceScale_;
    QDoubleSpinBox * timeScale_;
    QCheckBox * isTimeHorizontal_;
    QCheckBox * freezeSpaceRect_;
    QCheckBox * cameraFollowActiveTime_;

    QCheckBox * drawGrid_;
    QCheckBox * drawTimePlane_;
    QCheckBox * drawCurrentFrame_;
    QCheckBox * drawAllFrames_;
    QCheckBox * drawFramesAsTopology_;
    QCheckBox * drawCurrentFrameAsTopology_;
    QCheckBox * drawTopologyFaces_;
    QCheckBox * drawKeyCells_;
    QCheckBox * drawInbetweenCells_;
    QCheckBox * drawKeyVerticesAsDots_;
    QCheckBox * clipToSpaceTimeWindow_;

    QSpinBox * vertexTopologySize_;
    QSpinBox * edgeTopologyWidth_;

    QDoubleSpinBox * opacity_;
    QCheckBox * drawAsMesh_;
    QSpinBox * k1_;
    QSpinBox * k2_;
};

#endif // VIEW3DSETTINGS_H

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

#include "View3DSettings.h"

#include <QCloseEvent>

View3DSettings::View3DSettings() :
    // Display
    spaceScale_(0.001),
    timeScale_(0.011),
    isTimeHorizontal_(true),
    freezeSpaceRect_(false),
    cameraFollowActiveTime_(false),

    drawGrid_(false),
    drawTimePlane_(true),
    drawCurrentFrame_(true),
    drawAllFrames_(false),
    drawFramesAsTopology_(true),
    drawCurrentFrameAsTopology_(false),
    drawTopologyFaces_(true),
    drawKeyCells_(true),
    drawInbetweenCells_(true),
    drawKeyVerticesAsDots_(false),
    clipToSpaceTimeWindow_(false),

    vertexTopologySize_(5),
    edgeTopologyWidth_(3),

    opacity_(0.08),
    drawAsMesh_(true),
    k1_(1),
    k2_(1)
{
}

View3DSettings::~View3DSettings()
{
}

double View3DSettings::spaceScale() const
{
    return spaceScale_;
}
void View3DSettings::setSpaceScale(double newValue)
{
    spaceScale_ = newValue;
}

double View3DSettings::timeScale() const
{
    return timeScale_;
}
void View3DSettings::setTimeScale(double newValue)
{
    timeScale_ = newValue;
}

bool View3DSettings::isTimeHorizontal() const
{
    return isTimeHorizontal_;
}
void View3DSettings::setIsTimeHorizontal(bool newValue)
{
    isTimeHorizontal_ = newValue;
}

bool View3DSettings::freezeSpaceRect() const
{
    return freezeSpaceRect_;
}
void View3DSettings::setFreezeSpaceRect(bool newValue)
{
    freezeSpaceRect_ = newValue;
}

bool View3DSettings::cameraFollowActiveTime() const
{
    return cameraFollowActiveTime_;
}
void View3DSettings::setCameraFollowActiveTime(bool newValue)
{
    cameraFollowActiveTime_ = newValue;
}







bool View3DSettings::drawGrid() const
{
    return drawGrid_;
}
void View3DSettings::setDrawGrid(bool newValue)
{
    drawGrid_ = newValue;
}

bool View3DSettings::drawTimePlane() const
{
    return drawTimePlane_;
}
void View3DSettings::setDrawTimePlane(bool newValue)
{
    drawTimePlane_ = newValue;
}

bool View3DSettings::drawCurrentFrame() const
{
    return drawCurrentFrame_;
}
void View3DSettings::setDrawCurrentFrame(bool newValue)
{
    drawCurrentFrame_ = newValue;
}

bool View3DSettings::drawAllFrames() const
{
    return drawAllFrames_;
}
void View3DSettings::setDrawAllFrames(bool newValue)
{
    drawAllFrames_ = newValue;
}

bool View3DSettings::drawCurrentFrameAsTopology() const
{
    return drawCurrentFrameAsTopology_;
}
void View3DSettings::setDrawCurrentFrameAsTopology(bool newValue)
{
    drawCurrentFrameAsTopology_ = newValue;
}
bool View3DSettings::drawFramesAsTopology() const
{
    return drawFramesAsTopology_;
}
void View3DSettings::setDrawFramesAsTopology(bool newValue)
{
    drawFramesAsTopology_ = newValue;
}

bool View3DSettings::drawTopologyFaces() const
{
    return drawTopologyFaces_;
}
void View3DSettings::setDrawTopologyFaces(bool newValue)
{
    drawTopologyFaces_ = newValue;
}

bool View3DSettings::drawKeyCells() const
{
    return drawKeyCells_;
}
void View3DSettings::setDrawKeyCells(bool newValue)
{
    drawKeyCells_ = newValue;
}

bool View3DSettings::drawInbetweenCells() const
{
    return drawInbetweenCells_;
}
void View3DSettings::setDrawInbetweenCells(bool newValue)
{
    drawInbetweenCells_ = newValue;
}

bool View3DSettings::drawKeyVerticesAsDots() const
{
    return drawKeyVerticesAsDots_;
}
void View3DSettings::setDrawKeyVerticesAsDots(bool newValue)
{
    drawKeyVerticesAsDots_ = newValue;
}

bool View3DSettings::clipToSpaceTimeWindow() const
{
    return clipToSpaceTimeWindow_;
}
void View3DSettings::setClipToSpaceTimeWindow(bool newValue)
{
    clipToSpaceTimeWindow_ = newValue;
}

int View3DSettings::vertexTopologySize() const
{
    return vertexTopologySize_;
}
void View3DSettings::setVertexTopologySize(int newValue)
{
    vertexTopologySize_ = newValue;
}

int View3DSettings::edgeTopologyWidth() const
{
    return edgeTopologyWidth_;
}
void View3DSettings::setEdgeTopologyWidth(int newValue)
{
    edgeTopologyWidth_ = newValue;
}


double View3DSettings::opacity() const
{
    return opacity_;
}
void View3DSettings::setOpacity(double newValue)
{
    opacity_ = newValue;
}
bool View3DSettings::drawAsMesh() const
{
    return drawAsMesh_;
}
void View3DSettings::setDrawAsMesh(bool newValue)
{
    drawAsMesh_ = newValue;
}
int View3DSettings::k1() const
{
    return k1_;
}
void View3DSettings::setK1(int newValue)
{
    k1_ = newValue;
}
int View3DSettings::k2() const
{
    return k2_;
}
void View3DSettings::setK2(int newValue)
{
    k2_ = newValue;
}


double View3DSettings::xFromX2D(double xScene) const
{
    return xScene;
}
double View3DSettings::yFromY2D(double yScene) const
{
    return -yScene;
}

double View3DSettings::zFromT(double time) const
{
    double res = - time * timeScale() / spaceScale();
    return    res;
}

double View3DSettings::zFromT(int time) const
{
    return zFromT( (double) time );
}

double View3DSettings::zFromT(Time time) const
{
    return zFromT(time.floatTime());
}

double View3DSettings::xSceneMin() const { return xSceneMin_; }
double View3DSettings::xSceneMax() const { return xSceneMax_; }
double View3DSettings::ySceneMin() const { return ySceneMin_; }
double View3DSettings::ySceneMax() const { return ySceneMax_; }

void View3DSettings::setXSceneMin(double value)
{
    xSceneMin_ = value;
}

void View3DSettings::setXSceneMax(double value)
{
    xSceneMax_ = value;
}

void View3DSettings::setYSceneMin(double value)
{
    ySceneMin_ = value;
}

void View3DSettings::setYSceneMax(double value)
{
    ySceneMax_ = value;
}



View3DSettingsWidget::View3DSettingsWidget(View3DSettings & viewSettings) :
    QWidget(0),
    viewSettings_(viewSettings)
{
    setWindowTitle("3D View Settings [Beta]");

    spaceScale_ = new QDoubleSpinBox();
    spaceScale_->setRange(-100,100);
    spaceScale_->setDecimals(5);
    spaceScale_->setSingleStep(0.0001);

    timeScale_ = new QDoubleSpinBox();
    timeScale_->setRange(-100,100);
    timeScale_->setDecimals(5);
    timeScale_->setSingleStep(0.001);

    isTimeHorizontal_ = new QCheckBox();
    freezeSpaceRect_ = new QCheckBox();
    cameraFollowActiveTime_ = new QCheckBox();

    drawGrid_ = new QCheckBox();
    drawTimePlane_ = new QCheckBox();
    drawCurrentFrame_ = new QCheckBox();
    drawAllFrames_ = new QCheckBox();
    drawFramesAsTopology_ = new QCheckBox();
    drawCurrentFrameAsTopology_ = new QCheckBox();
    drawTopologyFaces_ = new QCheckBox();
    drawKeyCells_ = new QCheckBox();
    drawInbetweenCells_ = new QCheckBox();
    drawKeyVerticesAsDots_ = new QCheckBox();
    clipToSpaceTimeWindow_ = new QCheckBox();

    vertexTopologySize_ = new QSpinBox();
    vertexTopologySize_->setRange(1,100);

    edgeTopologyWidth_ = new QSpinBox();
    edgeTopologyWidth_->setRange(1,100);


    opacity_ = new QDoubleSpinBox();
    opacity_->setRange(0,1);
    opacity_->setDecimals(3);
    opacity_->setSingleStep(0.05);
    drawAsMesh_ = new QCheckBox();
    k1_ = new QSpinBox();
    k1_->setRange(1,100);
    k2_ = new QSpinBox();
    k2_->setRange(1,100);

    QFormLayout * layout = new QFormLayout();
    layout->addRow("Space scale:", spaceScale_);
    layout->addRow("Time scale:", timeScale_);
    //layout->addRow("Is time horizontal:", isTimeHorizontal_);
    layout->addRow("Freeze space rect:", freezeSpaceRect_);
    layout->addRow("Camera follow active time:", cameraFollowActiveTime_);
    layout->addRow("Draw grid:", drawGrid_);
    layout->addRow("Draw canvas:", drawTimePlane_);
    layout->addRow("Draw current frame:", drawCurrentFrame_);
    layout->addRow("Draw current frame as topology:", drawCurrentFrameAsTopology_);
    layout->addRow("Draw faces:", drawTopologyFaces_);
    layout->addRow("Draw all frames:", drawAllFrames_);
    layout->addRow("Draw frames as topology:", drawFramesAsTopology_);
    layout->addRow("Draw key cells:", drawKeyCells_);
    layout->addRow("Draw inbetween cells:", drawInbetweenCells_);
    //layout->addRow("Draw vertices as dots:", drawKeyVerticesAsDots_);
    //layout->addRow("clip to space-time window:", clipToSpaceTimeWindow_);
    layout->addRow("Vertex topology size:", vertexTopologySize_);
    layout->addRow("Edge topology width:", edgeTopologyWidth_);
    layout->addRow("Opacity:", opacity_);
    layout->addRow("Draw inbetween faces as mesh:", drawAsMesh_);
    layout->addRow("Mesh temporal res:", k1_);
    layout->addRow("Mesh inverse spatial res:", k2_);
    setLayout(layout);

    updateWidgetFromSettings(); // Might not be an exact match due to widget min/max values
    updateSettingsFromWidget(); // Make sure its an exact match

    connect(spaceScale_, SIGNAL(valueChanged(double)), this, SLOT(updateSettingsFromWidget()));
    connect(timeScale_, SIGNAL(valueChanged(double)), this, SLOT(updateSettingsFromWidget()));
    connect(isTimeHorizontal_, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(freezeSpaceRect_, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(cameraFollowActiveTime_, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(drawGrid_, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(drawTimePlane_, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(drawCurrentFrame_, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(drawCurrentFrameAsTopology_, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(drawTopologyFaces_, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(drawAllFrames_, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(drawFramesAsTopology_, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(drawKeyCells_, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(drawInbetweenCells_, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(drawKeyVerticesAsDots_, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(clipToSpaceTimeWindow_, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(vertexTopologySize_, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(edgeTopologyWidth_, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(opacity_, SIGNAL(valueChanged(double)), this, SLOT(updateSettingsFromWidget()));
    connect(drawAsMesh_, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(k1_, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(k2_, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidget()));
}

View3DSettingsWidget::~View3DSettingsWidget()
{
}

void View3DSettingsWidget::closeEvent(QCloseEvent * event)
{
    emit closed();
    event->accept();
}

void View3DSettingsWidget::updateWidgetFromSettings()
{
    spaceScale_->setValue(viewSettings_.spaceScale());
    timeScale_->setValue(viewSettings_.timeScale());
    isTimeHorizontal_->setChecked(viewSettings_.isTimeHorizontal());
    freezeSpaceRect_->setChecked(viewSettings_.freezeSpaceRect());
    cameraFollowActiveTime_->setChecked(viewSettings_.cameraFollowActiveTime());
    drawGrid_->setChecked(viewSettings_.drawGrid());
    drawTimePlane_->setChecked(viewSettings_.drawTimePlane());
    drawCurrentFrame_->setChecked(viewSettings_.drawCurrentFrame());
    drawAllFrames_->setChecked(viewSettings_.drawAllFrames());
    drawFramesAsTopology_->setChecked(viewSettings_.drawFramesAsTopology());
    drawCurrentFrameAsTopology_->setChecked(viewSettings_.drawCurrentFrameAsTopology());
    drawTopologyFaces_->setChecked(viewSettings_.drawTopologyFaces());
    drawKeyCells_->setChecked(viewSettings_.drawKeyCells());
    drawInbetweenCells_->setChecked(viewSettings_.drawInbetweenCells());
    drawKeyVerticesAsDots_->setChecked(viewSettings_.drawKeyVerticesAsDots());
    clipToSpaceTimeWindow_->setChecked(viewSettings_.clipToSpaceTimeWindow());
    vertexTopologySize_->setValue(viewSettings_.vertexTopologySize());
    edgeTopologyWidth_->setValue(viewSettings_.edgeTopologyWidth());
    opacity_->setValue(viewSettings_.opacity());
    drawAsMesh_->setChecked(viewSettings_.drawAsMesh());
    k1_->setValue(viewSettings_.k1());
    k2_->setValue(viewSettings_.k2());
}

void View3DSettingsWidget::updateSettingsFromWidget()
{
    viewSettings_.setSpaceScale(spaceScale_->value());
    viewSettings_.setTimeScale(timeScale_->value());
    viewSettings_.setIsTimeHorizontal(isTimeHorizontal_->isChecked());
    viewSettings_.setFreezeSpaceRect(freezeSpaceRect_->isChecked());
    viewSettings_.setCameraFollowActiveTime(cameraFollowActiveTime_->isChecked());
    viewSettings_.setDrawGrid(drawGrid_->isChecked());
    viewSettings_.setDrawTimePlane(drawTimePlane_->isChecked());
    viewSettings_.setDrawCurrentFrame(drawCurrentFrame_->isChecked());
    viewSettings_.setDrawAllFrames(drawAllFrames_->isChecked());
    viewSettings_.setDrawFramesAsTopology(drawFramesAsTopology_->isChecked());
    viewSettings_.setDrawCurrentFrameAsTopology(drawCurrentFrameAsTopology_->isChecked());
    viewSettings_.setDrawTopologyFaces(drawTopologyFaces_->isChecked());
    viewSettings_.setDrawKeyCells(drawKeyCells_->isChecked());
    viewSettings_.setDrawInbetweenCells(drawInbetweenCells_->isChecked());
    viewSettings_.setDrawKeyVerticesAsDots(drawKeyVerticesAsDots_->isChecked());
    viewSettings_.setClipToSpaceTimeWindow(clipToSpaceTimeWindow_->isChecked());
    viewSettings_.setVertexTopologySize(vertexTopologySize_->value());
    viewSettings_.setEdgeTopologyWidth(edgeTopologyWidth_->value());
    viewSettings_.setOpacity(opacity_->value());
    viewSettings_.setDrawAsMesh(drawAsMesh_->isChecked());
    viewSettings_.setK1(k1_->value());
    viewSettings_.setK2(k2_->value());

    emit changed();
}

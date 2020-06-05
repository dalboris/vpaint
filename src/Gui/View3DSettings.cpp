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

#include "View3DSettings.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QGroupBox>

#include "Global.h"

View3DSettings::View3DSettings() :
    // Display
    spaceScale_(0.001),
    timeScale_(0.010),
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
    k2_(1),

    pngWidth_(1920),
    pngHeight_(1080)
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

int View3DSettings::pngWidth() const
{
    return pngWidth_;
}

void View3DSettings::setPngWidth(int newValue)
{
    pngWidth_ = newValue;
}

int View3DSettings::pngHeight() const
{
    return pngHeight_;
}

void View3DSettings::setPngHeight(int newValue)
{
    pngHeight_ = newValue;
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



View3DSettingsWidget::View3DSettingsWidget() :
    QWidget(0),
    viewSettings_(nullptr),
    isUpdatingWidgetFromSettings_(false)
{
    setWindowTitle("3D View Settings [Beta]");
    setMinimumWidth(400);

    // Main layout, combining all groups
    QVBoxLayout * mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    // Space-time scales
    QGroupBox * scalesGroupBox = new QGroupBox(tr("Space-time scales"));
    QFormLayout * scalesLayout = new QFormLayout();
    scalesGroupBox->setLayout(scalesLayout);
    spaceScale_ = new QDoubleSpinBox();
    spaceScale_->setRange(-100,100);
    spaceScale_->setDecimals(5);
    spaceScale_->setSingleStep(0.0001);
    spaceScale_->setMaximumWidth(80);
    scalesLayout->addRow("Space scale:", spaceScale_);
    timeScale_ = new QDoubleSpinBox();
    timeScale_->setRange(-100,100);
    timeScale_->setDecimals(5);
    timeScale_->setSingleStep(0.001);
    timeScale_->setMaximumWidth(80);
    scalesLayout->addRow("Time scale:", timeScale_);
    mainLayout->addWidget(scalesGroupBox);

    isTimeHorizontal_ = new QCheckBox(); // XXX to delete
    freezeSpaceRect_ = new QCheckBox(); // XXX to delete

    // Camera behavior
    QGroupBox * cameraGroupBox = new QGroupBox(tr("Camera"));
    QVBoxLayout * cameraLayout = new QVBoxLayout();
    cameraGroupBox->setLayout(cameraLayout);
    cameraFollowActiveTime_ = new QCheckBox("Follow current frame");
    cameraLayout->addWidget(cameraFollowActiveTime_);
    mainLayout->addWidget(cameraGroupBox);

    drawGrid_ = new QCheckBox(); // XXX to delete

    // 2D frames
    QGroupBox * frames2dGroupBox = new QGroupBox(tr("2D frames"));
    QVBoxLayout * frames2dLayout = new QVBoxLayout();
    frames2dGroupBox->setLayout(frames2dLayout);
    drawCurrentFrame_ = new QCheckBox("Draw current frame");
    frames2dLayout->addWidget(drawCurrentFrame_);
    drawTimePlane_ = new QCheckBox("Draw current frame's canvas");
    frames2dLayout->addWidget(drawTimePlane_);
    drawKeyCells_ = new QCheckBox("Draw all key cells");
    frames2dLayout->addWidget(drawKeyCells_);
    drawAllFrames_ = new QCheckBox("Draw all frames");
    frames2dLayout->addWidget(drawAllFrames_);
    drawCurrentFrameAsTopology_ = new QCheckBox("Draw current frame as outline");
    frames2dLayout->addWidget(drawCurrentFrameAsTopology_);
    drawFramesAsTopology_ = new QCheckBox("Draw other frames as outline");
    frames2dLayout->addWidget(drawFramesAsTopology_);
    drawTopologyFaces_ = new QCheckBox("Draw faces even for frames drawn as outline");
    frames2dLayout->addWidget(drawTopologyFaces_);
    drawKeyVerticesAsDots_ = new QCheckBox(); // XXX to delete
    clipToSpaceTimeWindow_ = new QCheckBox(); // XXX to delete
    QFormLayout * outlineSizeLayout = new QFormLayout();
    vertexTopologySize_ = new QSpinBox();
    vertexTopologySize_->setRange(1,100);
    vertexTopologySize_->setMaximumWidth(80);
    outlineSizeLayout->addRow("Vertex outline size:", vertexTopologySize_);
    edgeTopologyWidth_ = new QSpinBox();
    edgeTopologyWidth_->setRange(1,100);
    edgeTopologyWidth_->setMaximumWidth(80);
    outlineSizeLayout->addRow("Edge outline width:", edgeTopologyWidth_);
    frames2dLayout->addLayout(outlineSizeLayout);
    mainLayout->addWidget(frames2dGroupBox);

    // 3D space-time mesh
    QGroupBox * frames3dGroupBox = new QGroupBox(tr("3D space-time mesh"));
    QVBoxLayout * frames3dLayout = new QVBoxLayout();
    frames3dGroupBox->setLayout(frames3dLayout);
    drawInbetweenCells_ = new QCheckBox("Draw space-time mesh");
    frames3dLayout->addWidget(drawInbetweenCells_);
    drawAsMesh_ = new QCheckBox("Draw as lines");
    frames3dLayout->addWidget(drawAsMesh_);
    QFormLayout * meshLayout = new QFormLayout();
    opacity_ = new QDoubleSpinBox();
    opacity_->setRange(0,1);
    opacity_->setDecimals(3);
    opacity_->setSingleStep(0.05);
    opacity_->setMaximumWidth(80);
    k1_ = new QSpinBox();
    k1_->setRange(1,100);
    k1_->setMaximumWidth(80);
    k2_ = new QSpinBox();
    k2_->setRange(1,100);
    k2_->setMaximumWidth(80);
    meshLayout->addRow("Opacity:", opacity_);
    meshLayout->addRow("Temporal resolution:", k1_);
    meshLayout->addRow("Inverse spatial resolution:", k2_);
    frames3dLayout->addLayout(meshLayout);
    mainLayout->addWidget(frames3dGroupBox);

    // Export settings and button
    QGroupBox * exportGroupBox = new QGroupBox(tr("Export as PNG (image or sequence)"));
    QVBoxLayout * exportLayout = new QVBoxLayout();
    exportGroupBox->setLayout(exportLayout);
    QFormLayout * pngFormLayout = new QFormLayout();
    pngWidth_ = new QSpinBox();
    pngWidth_->setRange(1, 10000);
    pngWidth_->setMaximumWidth(80);
    pngFormLayout->addRow("Width:", pngWidth_);
    pngHeight_ = new QSpinBox();
    pngHeight_->setRange(1, 10000);
    pngHeight_->setMaximumWidth(80);
    pngFormLayout->addRow("Height:", pngHeight_);
    QHBoxLayout * exportFilenameLayout = new QHBoxLayout();
    exportFilename_ = new QLineEdit();
    exportFilenameLayout->addWidget(exportFilename_);
    exportBrowseButton_ = new QPushButton("Browse...");
    exportFilenameLayout->addWidget(exportBrowseButton_);
    pngFormLayout->addRow("Filename: ", exportFilenameLayout);
    exportLayout->addLayout(pngFormLayout);
    exportButton_ = new QPushButton("Export");
    exportButton_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    exportLayout->addWidget(exportButton_);
    mainLayout->addWidget(exportGroupBox);

    // Prevent group boxes from stretching vertically.
    // Instead, we just add blank space at the bottom.
    mainLayout->addStretch();

    // Connections
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
    connect(pngWidth_, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(pngHeight_, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidget()));

    connect(exportBrowseButton_, SIGNAL(clicked()), this, SLOT(onExportBrowseButtonClicked()));
    connect(exportButton_, SIGNAL(clicked()), this, SLOT(onExportButtonClicked()));
}

View3DSettingsWidget::~View3DSettingsWidget()
{

}

void View3DSettingsWidget::setViewSettings(View3DSettings * viewSettings)
{
    viewSettings_ = viewSettings;

    updateWidgetFromSettings(); // Might not be an exact match due to widget min/max values
    updateSettingsFromWidget(); // Make sure its an exact match
}

QString View3DSettingsWidget::exportFilename() const
{
    return exportFilename_->text();
}

void View3DSettingsWidget::closeEvent(QCloseEvent * event)
{
    emit closed();
    event->accept();
}

void View3DSettingsWidget::updateWidgetFromSettings()
{
    isUpdatingWidgetFromSettings_ = true;

    if (viewSettings_)
    {
        spaceScale_->setValue(viewSettings_->spaceScale());
        timeScale_->setValue(viewSettings_->timeScale());
        isTimeHorizontal_->setChecked(viewSettings_->isTimeHorizontal());
        freezeSpaceRect_->setChecked(viewSettings_->freezeSpaceRect());
        cameraFollowActiveTime_->setChecked(viewSettings_->cameraFollowActiveTime());
        drawGrid_->setChecked(viewSettings_->drawGrid());
        drawTimePlane_->setChecked(viewSettings_->drawTimePlane());
        drawCurrentFrame_->setChecked(viewSettings_->drawCurrentFrame());
        drawAllFrames_->setChecked(viewSettings_->drawAllFrames());
        drawFramesAsTopology_->setChecked(viewSettings_->drawFramesAsTopology());
        drawCurrentFrameAsTopology_->setChecked(viewSettings_->drawCurrentFrameAsTopology());
        drawTopologyFaces_->setChecked(viewSettings_->drawTopologyFaces());
        drawKeyCells_->setChecked(viewSettings_->drawKeyCells());
        drawInbetweenCells_->setChecked(viewSettings_->drawInbetweenCells());
        drawKeyVerticesAsDots_->setChecked(viewSettings_->drawKeyVerticesAsDots());
        clipToSpaceTimeWindow_->setChecked(viewSettings_->clipToSpaceTimeWindow());
        vertexTopologySize_->setValue(viewSettings_->vertexTopologySize());
        edgeTopologyWidth_->setValue(viewSettings_->edgeTopologyWidth());
        opacity_->setValue(viewSettings_->opacity());
        drawAsMesh_->setChecked(viewSettings_->drawAsMesh());
        k1_->setValue(viewSettings_->k1());
        k2_->setValue(viewSettings_->k2());
        pngWidth_->setValue(viewSettings_->pngWidth());
        pngHeight_->setValue(viewSettings_->pngHeight());
    }

    isUpdatingWidgetFromSettings_ = false;
}

void View3DSettingsWidget::updateSettingsFromWidget()
{
    if (!isUpdatingWidgetFromSettings_ && viewSettings_)
    {
        viewSettings_->setSpaceScale(spaceScale_->value());
        viewSettings_->setTimeScale(timeScale_->value());
        viewSettings_->setIsTimeHorizontal(isTimeHorizontal_->isChecked());
        viewSettings_->setFreezeSpaceRect(freezeSpaceRect_->isChecked());
        viewSettings_->setCameraFollowActiveTime(cameraFollowActiveTime_->isChecked());
        viewSettings_->setDrawGrid(drawGrid_->isChecked());
        viewSettings_->setDrawTimePlane(drawTimePlane_->isChecked());
        viewSettings_->setDrawCurrentFrame(drawCurrentFrame_->isChecked());
        viewSettings_->setDrawAllFrames(drawAllFrames_->isChecked());
        viewSettings_->setDrawFramesAsTopology(drawFramesAsTopology_->isChecked());
        viewSettings_->setDrawCurrentFrameAsTopology(drawCurrentFrameAsTopology_->isChecked());
        viewSettings_->setDrawTopologyFaces(drawTopologyFaces_->isChecked());
        viewSettings_->setDrawKeyCells(drawKeyCells_->isChecked());
        viewSettings_->setDrawInbetweenCells(drawInbetweenCells_->isChecked());
        viewSettings_->setDrawKeyVerticesAsDots(drawKeyVerticesAsDots_->isChecked());
        viewSettings_->setClipToSpaceTimeWindow(clipToSpaceTimeWindow_->isChecked());
        viewSettings_->setVertexTopologySize(vertexTopologySize_->value());
        viewSettings_->setEdgeTopologyWidth(edgeTopologyWidth_->value());
        viewSettings_->setOpacity(opacity_->value());
        viewSettings_->setDrawAsMesh(drawAsMesh_->isChecked());
        viewSettings_->setK1(k1_->value());
        viewSettings_->setK2(k2_->value());
        viewSettings_->setPngWidth(pngWidth_->value());
        viewSettings_->setPngHeight(pngHeight_->value());

        emit changed();
    }
}

void View3DSettingsWidget::onExportBrowseButtonClicked()
{
    QString initialDir;
    if (exportFilename_->text().isEmpty()) {
        initialDir = global()->documentDir().path();
    }
    else {
        initialDir = QFileInfo(exportFilename_->text()).dir().path();
    }

    QString filename = QFileDialog::getSaveFileName(
                this, tr("Export filename"), initialDir);

    if (!filename.isEmpty()) {
        if(!filename.endsWith(".png")) {
            filename.append(".png");
        }
        exportFilename_->setText(filename);
    }
}

void View3DSettingsWidget::onExportButtonClicked()
{
    emit exportClicked();
}

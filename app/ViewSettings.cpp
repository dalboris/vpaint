// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "ViewSettings.h"

ViewSettings::ViewSettings() :
    // Display
    zoom_(1.0),
    displayMode_(ILLUSTRATION),
    outlineOnly_(false),
    drawBackground_(true),
    drawCursor_(true),
    vertexTopologySize_(5),
    edgeTopologyWidth_(3),
    drawTopologyFaces_(false),
    screenRelative_(true),

    time_(),

    // Onion skinning
    onionSkinningIsEnabled_(false),
    areOnionSkinsPickable_(true),
    numOnionSkinsBefore_(3),
    numOnionSkinsAfter_(3),
    onionSkinsTimeOffset_(Time(1)),
    onionSkinsXOffset_(0.0),
    onionSkinsYOffset_(0.0),
    onionSkinsTransparencyRatio_(1.0)
{
}

ViewSettings::~ViewSettings()
{
}


Time ViewSettings::time() const
{
    return time_;
}

void ViewSettings::setTime(const Time & t)
{
    time_ = t;
}

// Zoom level
double ViewSettings::zoom() const
{
    return zoom_;
}
void ViewSettings::setZoom(double newValue)
{
    if(zoom_ != newValue)
    {
        zoom_ = newValue;
    }
}

ViewSettings::DisplayMode ViewSettings::displayMode() const
{
    return displayMode_;
}
void ViewSettings::setDisplayMode(DisplayMode mode)
{
    if(displayMode_ != mode)
    {
        displayMode_ = mode;
    }
}

void ViewSettings::toggleOutline()
{
    if(displayMode() == ILLUSTRATION)
    {
        if(outlineOnly_)
            setDisplayMode(OUTLINE);
        else
            setDisplayMode(ILLUSTRATION_OUTLINE);
    }
    else
        setDisplayMode(ILLUSTRATION);
}

void ViewSettings::toggleOutlineOnly()
{
    if(displayMode() != ILLUSTRATION)
    {
        outlineOnly_ = !outlineOnly_;
        if(outlineOnly_)
            setDisplayMode(OUTLINE);
        else
            setDisplayMode(ILLUSTRATION_OUTLINE);
    }
}

bool ViewSettings::drawBackground() const
{
    return drawBackground_;
}
void ViewSettings::setDrawBackground(bool newValue)
{
    if(drawBackground_ != newValue)
    {
        drawBackground_ = newValue;
    }
}

bool ViewSettings::drawCursor() const
{
    return drawCursor_;
}
void ViewSettings::setDrawCursor(bool newValue)
{
    drawCursor_ = newValue;
}

bool ViewSettings::isMainDrawing() const
{
    return isMainDrawing_;
}
void ViewSettings::setMainDrawing(bool newValue)
{
    if(isMainDrawing_ != newValue)
    {
        isMainDrawing_ = newValue;
    }
}

int ViewSettings::vertexTopologySize() const
{
    return vertexTopologySize_;
}
void ViewSettings::setVertexTopologySize(int newValue)
{
    if(vertexTopologySize_ != newValue)
    {
        vertexTopologySize_ = newValue;
    }
}

int ViewSettings::edgeTopologyWidth() const
{
    return edgeTopologyWidth_;
}
void ViewSettings::setEdgeTopologyWidth(int newValue)
{
    if(edgeTopologyWidth_ != newValue)
    {
        edgeTopologyWidth_ = newValue;
    }
}

bool ViewSettings::drawTopologyFaces() const
{
    return drawTopologyFaces_;
}
void ViewSettings::setDrawTopologyFaces(bool newValue)
{
    if(drawTopologyFaces_ != newValue)
    {
        drawTopologyFaces_ = newValue;
    }
}

bool ViewSettings::screenRelative() const
{
    return screenRelative_;
}
void ViewSettings::setScreenRelative(bool newValue)
{
    if(screenRelative_ != newValue)
    {
        screenRelative_ = newValue;
    }
}





bool ViewSettings::onionSkinningIsEnabled() const
{
    return onionSkinningIsEnabled_;
}
void ViewSettings::setOnionSkinningIsEnabled(bool newValue)
{
    onionSkinningIsEnabled_ = newValue;
}

bool ViewSettings::areOnionSkinsPickable() const
{
    return areOnionSkinsPickable_;
}
void ViewSettings::setAreOnionSkinsPickable(bool newValue)
{
    areOnionSkinsPickable_ = newValue;
}

int ViewSettings::numOnionSkinsBefore() const
{
    return numOnionSkinsBefore_;
}
void ViewSettings::setNumOnionSkinsBefore(int newValue)
{
    numOnionSkinsBefore_ = newValue;
}

int ViewSettings::numOnionSkinsAfter() const
{
    return numOnionSkinsAfter_;
}
void ViewSettings::setNumOnionSkinsAfter(int newValue)
{
    numOnionSkinsAfter_ = newValue;
}

Time ViewSettings::onionSkinsTimeOffset() const
{
    return onionSkinsTimeOffset_;
}
void ViewSettings::setOnionSkinsTimeOffset(Time newValue)
{
    onionSkinsTimeOffset_ = newValue;
}

void ViewSettings::setOnionSkinsTimeOffset(double newValue)
{
    int intValue = newValue;
    if(newValue == (double) intValue)
        onionSkinsTimeOffset_ = Time(intValue); // Exact frame
    else
        onionSkinsTimeOffset_ = Time(newValue); // float time
}

double ViewSettings::onionSkinsXOffset() const
{
    return onionSkinsXOffset_;
}
void ViewSettings::setOnionSkinsXOffset(double newValue)
{
    onionSkinsXOffset_ = newValue;
}

double ViewSettings::onionSkinsYOffset() const
{
    return onionSkinsYOffset_;
}
void ViewSettings::setOnionSkinsYOffset(double newValue)
{
    onionSkinsYOffset_ = newValue;
}

double ViewSettings::onionSkinsTransparencyRatio() const
{
    return onionSkinsTransparencyRatio_;
}
void ViewSettings::setOnionSkinsTransparencyRatio(double newValue)
{
    onionSkinsTransparencyRatio_ = newValue;
}

#include <QWidgetAction>
#include <QMenuBar>
#include <QMenu>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>

ViewSettingsWidget::ViewSettingsWidget(ViewSettings & viewSettings, QWidget * parent) :
    QWidget(parent),
    viewSettings_(viewSettings),
    ignoreZoomValueChangedSignal_(false)
{
    // ----------------- MacOS X style ------------------------

#ifdef Q_OS_MAC
    // Set style
    setAutoFillBackground(true);
    setCursor(Qt::ArrowCursor);
    buttonSize = 48;

    /* THE FOLLOWING ARE NOT IMPLEMENTED OR DEEMED IRRELEVANT
    screenRelative_ = new QCheckBox();
    onionSkinsTransparencyRatio_ = new QDoubleSpinBox();
    onionSkinsTransparencyRatio_->setRange(0.0,1.0);
    onionSkinsTransparencyRatio_->setDecimals(2);
    */

    // Button to show/hide view settings

    showHideSettingsButton_ = new QPushButton("");
    showHideSettingsButton_->setToolTip("Show/hide view settings");
    showHideSettingsButton_->setIcon(QIcon(":images/view-settings.png"));
    //showHideSettingsButton_->setIconSize(QSize(32,32));
    showHideSettingsButton_->setCheckable(true);
    showHideSettingsButton_->setChecked(true);
    showHideSettingsButton_->setFixedSize(20,20);
    showHideSettingsButton_->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    connect(showHideSettingsButton_, SIGNAL(toggled(bool)), this, SLOT(toggleVisible(bool)));


    // View label, frame number, zoom level

    //QGridLayout * frameZoomLayout = new QGridLayout();
    //frameZoomLayout->setMargin(0);
    //frameZoomLayout->setSpacing(0);

    int frameZoomWidth = buttonSize+30;

    QPushButton * goToPreviousFrameButton = new QPushButton();
    goToPreviousFrameButton->setFixedSize(20,20);
    goToPreviousFrameButton->setIcon(QIcon(":/images/go-first-view.png"));
    goToPreviousFrameButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    //frameZoomLayout->addWidget(goToPreviousFrameButton,0,0);

    QPushButton * goToNextFrameButton = new QPushButton();
    goToNextFrameButton->setFixedSize(20,20);
    goToNextFrameButton->setIcon(QIcon(":/images/go-last-view.png"));
    goToNextFrameButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    //frameZoomLayout->addWidget(goToNextFrameButton,0,2);

    frameLineEdit_ = new QLineEdit();
    frameLineEdit_->setFixedSize(40,20);
    frameLineEdit_->setAlignment(Qt::AlignCenter);
    frameLineEdit_->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    //frameZoomLayout->addWidget(frameLineEdit_,0,1);

    zoomSpinBox_ = new QSpinBox();
    zoomSpinBox_->setRange(1,1e7);
    zoomSpinBox_->setValue(100);
    zoomSpinBox_->setSuffix(" %");
    zoomSpinBox_->setFixedSize(75,20);
    zoomSpinBox_->setAlignment(Qt::AlignCenter);
    zoomSpinBox_->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    //zoomSpinBox_->setDecimals(0);
    //frameZoomLayout->addWidget(zoomSpinBox_,1,0,1,3);


    // Menu to change the display mode and options

    QHBoxLayout * displayModeLayout = new QHBoxLayout();
    displayModeLayout->setMargin(0);
    displayModeLayout->setSpacing(0);

    QVBoxLayout * displayModeLayoutLeftColumn = new QVBoxLayout();
    displayModeLayoutLeftColumn->setMargin(0);
    displayModeLayoutLeftColumn->setSpacing(0);

    QFormLayout * displayModeLayoutRightColumn = new QFormLayout();
    displayModeLayoutRightColumn->setMargin(10);
    displayModeLayoutRightColumn->setSpacing(10);

    displayModeButton_Normal_ = new QPushButton();
    displayModeButton_Normal_->setToolTip("Normal display mode");
    displayModeButton_Normal_->setIcon(QIcon(":images/display-mode-normal.png"));
    displayModeButton_Normal_->setIconSize(QSize(32,32));
    displayModeButton_Normal_->setFixedSize(buttonSize+20,buttonSize);
    displayModeButton_Normal_->setAutoExclusive(true);
    displayModeButton_Normal_->setCheckable(true);
    displayModeButton_Normal_->setChecked(true);

    displayModeButton_NormalOutline_ = new QPushButton();
    displayModeButton_NormalOutline_->setToolTip("Normal+Outline display mode");
    displayModeButton_NormalOutline_->setIcon(QIcon(":images/display-mode-normal-outline.png"));
    displayModeButton_NormalOutline_->setIconSize(QSize(32,32));
    displayModeButton_NormalOutline_->setFixedSize(buttonSize+20,buttonSize);
    displayModeButton_NormalOutline_->setAutoExclusive(true);
    displayModeButton_NormalOutline_->setCheckable(true);
    displayModeButton_NormalOutline_->setChecked(false);

    displayModeButton_Outline_ = new QPushButton();
    displayModeButton_Outline_->setToolTip("Outline display mode");
    displayModeButton_Outline_->setIcon(QIcon(":images/display-mode-outline.png"));
    displayModeButton_Outline_->setIconSize(QSize(32,32));
    displayModeButton_Outline_->setFixedSize(buttonSize+20,buttonSize);
    displayModeButton_Outline_->setAutoExclusive(true);
    displayModeButton_Outline_->setCheckable(true);
    displayModeButton_Outline_->setChecked(false);

    displayModeLayoutLeftColumn->addWidget(displayModeButton_Normal_);
    displayModeLayoutLeftColumn->addWidget(displayModeButton_NormalOutline_);
    displayModeLayoutLeftColumn->addWidget(displayModeButton_Outline_);
    displayModeLayoutLeftColumn->addStretch();


    vertexTopologySize_ = new QSlider();
    vertexTopologySize_->setOrientation(Qt::Horizontal);
    vertexTopologySize_->setRange(0,100);
    displayModeLayoutRightColumn->addRow(tr("Vertex outline size"), vertexTopologySize_);

    edgeTopologyWidth_ = new QSlider();
    edgeTopologyWidth_->setOrientation(Qt::Horizontal);
    edgeTopologyWidth_->setRange(0,100);
    displayModeLayoutRightColumn->addRow(tr("Edge outline width"), edgeTopologyWidth_);

    drawTopologyFaces_ = new QCheckBox();
    displayModeLayoutRightColumn->addRow(tr("Display faces in outline mode"), drawTopologyFaces_);

    QWidget * displayModeWidget = new QWidget();
    displayModeLayout->addLayout(displayModeLayoutLeftColumn);
    displayModeLayout->addLayout(displayModeLayoutRightColumn);
    displayModeWidget->setLayout(displayModeLayout);

    QMenu * displayModeMenu = new QMenu();
    QWidgetAction * displayModeWidgetAction = new QWidgetAction(this);
    displayModeWidgetAction->setDefaultWidget(displayModeWidget);
    displayModeMenu->addAction(displayModeWidgetAction);


    // Button to show/hide the display mode menu

    displayModeButton_ = new QPushButton();
    displayModeButton_->setToolTip("Display mode settings");
    displayModeButton_->setIcon(QIcon(":images/display-mode-normal.png"));
    //displayModeButton_->setIconSize(QSize(32,32));
    displayModeButton_->setFixedSize(40,20);
    displayModeButton_->setMenu(displayModeMenu);
    displayModeButton_->setAttribute(Qt::WA_LayoutUsesWidgetRect);


    // Menu to change onion skinning options

    QHBoxLayout * onionSkinningLayout = new QHBoxLayout();
    onionSkinningLayout->setMargin(0);
    onionSkinningLayout->setSpacing(0);

    QVBoxLayout * onionSkinningLayoutLeftColumn = new QVBoxLayout();
    onionSkinningLayoutLeftColumn->setMargin(0);
    onionSkinningLayoutLeftColumn->setSpacing(0);

    QFormLayout * onionSkinningLayoutRightColumn = new QFormLayout();
    onionSkinningLayoutRightColumn->setMargin(10);
    onionSkinningLayoutRightColumn->setSpacing(10);

    QFormLayout * onionSkinningLayoutRightColumn2 = new QFormLayout();
    onionSkinningLayoutRightColumn2->setMargin(10);
    onionSkinningLayoutRightColumn2->setSpacing(10);

    onionSkinningButton_Off_ = new QPushButton();
    onionSkinningButton_Off_->setToolTip("Onion skinning off");
    onionSkinningButton_Off_->setIcon(QIcon(":images/onion-skinning-off.png"));
    onionSkinningButton_Off_->setIconSize(QSize(32,32));
    onionSkinningButton_Off_->setFixedSize(buttonSize+20,buttonSize);
    onionSkinningButton_Off_->setAutoExclusive(true);
    onionSkinningButton_Off_->setCheckable(true);
    onionSkinningButton_Off_->setChecked(true);

    onionSkinningButton_On_ = new QPushButton();
    onionSkinningButton_On_->setToolTip("Onion skinning on");
    onionSkinningButton_On_->setIcon(QIcon(":images/onion-skinning-on.png"));
    onionSkinningButton_On_->setIconSize(QSize(32,32));
    onionSkinningButton_On_->setFixedSize(buttonSize+20,buttonSize);
    onionSkinningButton_On_->setAutoExclusive(true);
    onionSkinningButton_On_->setCheckable(true);
    onionSkinningButton_On_->setChecked(false);

    onionSkinningLayoutLeftColumn->addWidget(onionSkinningButton_Off_);
    onionSkinningLayoutLeftColumn->addWidget(onionSkinningButton_On_);
    onionSkinningLayoutLeftColumn->addStretch();

    numOnionSkinsBefore_ = new QSpinBox();
    numOnionSkinsBefore_->setRange(0,100);
    onionSkinningLayoutRightColumn->addRow(tr("Num skins before"), numOnionSkinsBefore_);

    numOnionSkinsAfter_ = new QSpinBox();
    numOnionSkinsAfter_->setRange(0,100);
    onionSkinningLayoutRightColumn->addRow(tr("Num skins after"), numOnionSkinsAfter_);

    areOnionSkinsPickable_ = new QCheckBox();
    onionSkinningLayoutRightColumn->addRow(tr("Are skins pickable"), areOnionSkinsPickable_);

    onionSkinsTimeOffset_ = new QDoubleSpinBox();
    onionSkinsTimeOffset_->setRange(-100,100);
    onionSkinsTimeOffset_->setDecimals(2);
    onionSkinningLayoutRightColumn2->addRow(tr("Skins frame offset"), onionSkinsTimeOffset_);

    onionSkinsXOffset_ = new QDoubleSpinBox();
    onionSkinsXOffset_->setRange(-10000,10000);
    onionSkinsXOffset_->setDecimals(2);
    onionSkinningLayoutRightColumn2->addRow(tr("Skin X offset"), onionSkinsXOffset_);

    onionSkinsYOffset_ = new QDoubleSpinBox();
    onionSkinsYOffset_->setRange(-10000,10000);
    onionSkinsYOffset_->setDecimals(2);
    onionSkinningLayoutRightColumn2->addRow(tr("Skin Y offset"), onionSkinsYOffset_);

    QWidget * onionSkinningWidget = new QWidget();
    onionSkinningLayout->addLayout(onionSkinningLayoutLeftColumn);
    onionSkinningLayout->addLayout(onionSkinningLayoutRightColumn);
    onionSkinningLayout->addLayout(onionSkinningLayoutRightColumn2);
    onionSkinningWidget->setLayout(onionSkinningLayout);

    QMenu * onionSkinningMenu = new QMenu();
    QWidgetAction * onionSkinningWidgetAction = new QWidgetAction(this);
    onionSkinningWidgetAction->setDefaultWidget(onionSkinningWidget);
    onionSkinningMenu->addAction(onionSkinningWidgetAction);


    // Button to show/hide the onion skinning menu

    onionSkinningButton_ = new QPushButton();
    onionSkinningButton_->setToolTip("Onion skinning settings");
    onionSkinningButton_->setIcon(QIcon(":images/display-mode-normal.png"));
    //onionSkinningButton_->setIconSize(QSize(32,32));
    onionSkinningButton_->setFixedSize(40,20);
    onionSkinningButton_->setMenu(onionSkinningMenu);
    onionSkinningButton_->setAttribute(Qt::WA_LayoutUsesWidgetRect);


    // Layouts and widgets for grouping

    QHBoxLayout * hlayout = new QHBoxLayout();
    hlayout->setMargin(0);
    hlayout->setSpacing(0);
    hlayout->addWidget(zoomSpinBox_);
    hlayout->addSpacing(5);
    hlayout->addWidget(goToPreviousFrameButton);
    hlayout->addWidget(frameLineEdit_);
    hlayout->addWidget(goToNextFrameButton);
    //hlayout->addLayout(frameZoomLayout);
    hlayout->addWidget(displayModeButton_);
    hlayout->addWidget(onionSkinningButton_);

    containerWidget = new QWidget();
    containerWidget->setLayout(hlayout);
    containerWidget->setVisible(true);

    QHBoxLayout * hlayoutfull = new QHBoxLayout();
    hlayoutfull->setMargin(0);
    hlayoutfull->setSpacing(0);
    hlayoutfull->addWidget(showHideSettingsButton_);
    hlayoutfull->addWidget(containerWidget);

    setLayout(hlayoutfull);


    // ----------------- Backend/Gui connections -----------------

    updateWidgetFromSettings(); // Might not be an exact match due to widget min/max values
    updateSettingsFromWidget(); // Make sure its an exact match

    /* THE FOLLOWING ARE NOT IMPLEMENTED OR DEEMED IRRELEVANT
    connect(screenRelative_, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(onionSkinsTransparencyRatio_, SIGNAL(valueChanged(double)), this, SLOT(updateSettingsFromWidget()));
    */

    // Widget -> Settings connection
    connect(zoomSpinBox_, SIGNAL(valueChanged(int)), this, SLOT(processZoomValueChangedSignal(int)));
    connect(frameLineEdit_, SIGNAL(returnPressed()), this,  SLOT(updateSettingsFromWidget()));
    connect(goToPreviousFrameButton, SIGNAL(clicked(bool)), this, SLOT(decrFrame()));
    connect(goToNextFrameButton, SIGNAL(clicked(bool)), this, SLOT(incrFrame()));

    connect(displayModeButton_Normal_, SIGNAL(clicked(bool)), this, SLOT(updateSettingsFromWidget()));
    connect(displayModeButton_NormalOutline_, SIGNAL(clicked(bool)), this, SLOT(updateSettingsFromWidget()));
    connect(displayModeButton_Outline_, SIGNAL(clicked(bool)), this, SLOT(updateSettingsFromWidget()));
    connect(onionSkinningButton_Off_, SIGNAL(clicked(bool)), this, SLOT(updateSettingsFromWidget()));
    connect(onionSkinningButton_On_, SIGNAL(clicked(bool)), this, SLOT(updateSettingsFromWidget()));

    connect(vertexTopologySize_, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(edgeTopologyWidth_, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(drawTopologyFaces_, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidget()));

    connect(areOnionSkinsPickable_, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(numOnionSkinsBefore_, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(numOnionSkinsAfter_, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(onionSkinsTimeOffset_, SIGNAL(valueChanged(double)), this, SLOT(updateSettingsFromWidget()));
    connect(onionSkinsXOffset_, SIGNAL(valueChanged(double)), this, SLOT(updateSettingsFromWidget()));
    connect(onionSkinsYOffset_, SIGNAL(valueChanged(double)), this, SLOT(updateSettingsFromWidget()));


    // Settings -> Widget connection
    //  -> must be done manually by owner of ViewSettings and ViewSettingsWidget


    // ----------------- Set position and size -----------------

    setGeometry(10,10,100,100); // Move to (10,10) and resize arbitrarily to (100,100)
    setFixedSize(sizeHint());   // Resize to optimal size



    // ----------------- Windows or Linux style ------------------------

#else
    // Set style
    setAutoFillBackground(true);
    setCursor(Qt::ArrowCursor);
    buttonSize = 48;

    /* THE FOLLOWING ARE NOT IMPLEMENTED OR DEEMED IRRELEVANT
    screenRelative_ = new QCheckBox();
    onionSkinsTransparencyRatio_ = new QDoubleSpinBox();
    onionSkinsTransparencyRatio_->setRange(0.0,1.0);
    onionSkinsTransparencyRatio_->setDecimals(2);
    */

    // Button to show/hide view settings

    showHideSettingsButton_ = new QPushButton("");
    showHideSettingsButton_->setToolTip("Show/hide view settings");
    showHideSettingsButton_->setIcon(QIcon(":images/view-settings.png"));
    showHideSettingsButton_->setIconSize(QSize(32,32));
    showHideSettingsButton_->setCheckable(true);
    showHideSettingsButton_->setChecked(true);
    showHideSettingsButton_->setFixedSize(buttonSize,buttonSize);
    connect(showHideSettingsButton_, SIGNAL(toggled(bool)), this, SLOT(toggleVisible(bool)));


    // View label, frame number, zoom level

    QGridLayout * frameZoomLayout = new QGridLayout();
    frameZoomLayout->setMargin(0);
    frameZoomLayout->setSpacing(0);

    int frameZoomWidth = buttonSize+30;

    QPushButton * goToPreviousFrameButton = new QPushButton();
    goToPreviousFrameButton->setFixedSize(25,0.5*buttonSize);
    goToPreviousFrameButton->setIcon(QIcon(":/images/go-first-view.png"));
    frameZoomLayout->addWidget(goToPreviousFrameButton,0,0);

    QPushButton * goToNextFrameButton = new QPushButton();
    goToNextFrameButton->setFixedSize(25,0.5*buttonSize);
    goToNextFrameButton->setIcon(QIcon(":/images/go-last-view.png"));
    frameZoomLayout->addWidget(goToNextFrameButton,0,2);

    frameLineEdit_ = new QLineEdit();
    frameLineEdit_->setFixedSize(frameZoomWidth-50,0.5*buttonSize);
    frameLineEdit_->setAlignment(Qt::AlignCenter);
    frameZoomLayout->addWidget(frameLineEdit_,0,1);

    zoomSpinBox_ = new QSpinBox();
    zoomSpinBox_->setRange(1,1e7);
    zoomSpinBox_->setValue(100);
    zoomSpinBox_->setSuffix(" %");
    zoomSpinBox_->setFixedSize(frameZoomWidth,0.5*buttonSize);
    zoomSpinBox_->setAlignment(Qt::AlignCenter);
    //zoomSpinBox_->setDecimals(0);
    frameZoomLayout->addWidget(zoomSpinBox_,1,0,1,3);


    // Menu to change the display mode and options

    QHBoxLayout * displayModeLayout = new QHBoxLayout();
    displayModeLayout->setMargin(0);
    displayModeLayout->setSpacing(0);

    QVBoxLayout * displayModeLayoutLeftColumn = new QVBoxLayout();
    displayModeLayoutLeftColumn->setMargin(0);
    displayModeLayoutLeftColumn->setSpacing(0);

    QFormLayout * displayModeLayoutRightColumn = new QFormLayout();
    displayModeLayoutRightColumn->setMargin(10);
    displayModeLayoutRightColumn->setSpacing(10);

    displayModeButton_Normal_ = new QPushButton();
    displayModeButton_Normal_->setToolTip("Normal display mode");
    displayModeButton_Normal_->setIcon(QIcon(":images/display-mode-normal.png"));
    displayModeButton_Normal_->setIconSize(QSize(32,32));
    displayModeButton_Normal_->setFixedSize(buttonSize+20,buttonSize);
    displayModeButton_Normal_->setAutoExclusive(true);
    displayModeButton_Normal_->setCheckable(true);
    displayModeButton_Normal_->setChecked(true);

    displayModeButton_NormalOutline_ = new QPushButton();
    displayModeButton_NormalOutline_->setToolTip("Normal+Outline display mode");
    displayModeButton_NormalOutline_->setIcon(QIcon(":images/display-mode-normal-outline.png"));
    displayModeButton_NormalOutline_->setIconSize(QSize(32,32));
    displayModeButton_NormalOutline_->setFixedSize(buttonSize+20,buttonSize);
    displayModeButton_NormalOutline_->setAutoExclusive(true);
    displayModeButton_NormalOutline_->setCheckable(true);
    displayModeButton_NormalOutline_->setChecked(false);

    displayModeButton_Outline_ = new QPushButton();
    displayModeButton_Outline_->setToolTip("Outline display mode");
    displayModeButton_Outline_->setIcon(QIcon(":images/display-mode-outline.png"));
    displayModeButton_Outline_->setIconSize(QSize(32,32));
    displayModeButton_Outline_->setFixedSize(buttonSize+20,buttonSize);
    displayModeButton_Outline_->setAutoExclusive(true);
    displayModeButton_Outline_->setCheckable(true);
    displayModeButton_Outline_->setChecked(false);

    displayModeLayoutLeftColumn->addWidget(displayModeButton_Normal_);
    displayModeLayoutLeftColumn->addWidget(displayModeButton_NormalOutline_);
    displayModeLayoutLeftColumn->addWidget(displayModeButton_Outline_);
    displayModeLayoutLeftColumn->addStretch();


    vertexTopologySize_ = new QSlider();
    vertexTopologySize_->setOrientation(Qt::Horizontal);
    vertexTopologySize_->setRange(0,100);
    displayModeLayoutRightColumn->addRow(tr("Vertex outline size"), vertexTopologySize_);

    edgeTopologyWidth_ = new QSlider();
    edgeTopologyWidth_->setOrientation(Qt::Horizontal);
    edgeTopologyWidth_->setRange(0,100);
    displayModeLayoutRightColumn->addRow(tr("Edge outline width"), edgeTopologyWidth_);

    drawTopologyFaces_ = new QCheckBox();
    displayModeLayoutRightColumn->addRow(tr("Display faces in outline mode"), drawTopologyFaces_);

    QWidget * displayModeWidget = new QWidget();
    displayModeLayout->addLayout(displayModeLayoutLeftColumn);
    displayModeLayout->addLayout(displayModeLayoutRightColumn);
    displayModeWidget->setLayout(displayModeLayout);

    QMenu * displayModeMenu = new QMenu();
    QWidgetAction * displayModeWidgetAction = new QWidgetAction(this);
    displayModeWidgetAction->setDefaultWidget(displayModeWidget);
    displayModeMenu->addAction(displayModeWidgetAction);


    // Button to show/hide the display mode menu

    displayModeButton_ = new QPushButton();
    displayModeButton_->setToolTip("Display mode settings");
    displayModeButton_->setIcon(QIcon(":images/display-mode-normal.png"));
    displayModeButton_->setIconSize(QSize(32,32));
    displayModeButton_->setFixedSize(buttonSize+20,buttonSize);
    displayModeButton_->setMenu(displayModeMenu);


    // Menu to change onion skinning options

    QHBoxLayout * onionSkinningLayout = new QHBoxLayout();
    onionSkinningLayout->setMargin(0);
    onionSkinningLayout->setSpacing(0);

    QVBoxLayout * onionSkinningLayoutLeftColumn = new QVBoxLayout();
    onionSkinningLayoutLeftColumn->setMargin(0);
    onionSkinningLayoutLeftColumn->setSpacing(0);

    QFormLayout * onionSkinningLayoutRightColumn = new QFormLayout();
    onionSkinningLayoutRightColumn->setMargin(10);
    onionSkinningLayoutRightColumn->setSpacing(10);

    QFormLayout * onionSkinningLayoutRightColumn2 = new QFormLayout();
    onionSkinningLayoutRightColumn2->setMargin(10);
    onionSkinningLayoutRightColumn2->setSpacing(10);

    onionSkinningButton_Off_ = new QPushButton();
    onionSkinningButton_Off_->setToolTip("Onion skinning off");
    onionSkinningButton_Off_->setIcon(QIcon(":images/onion-skinning-off.png"));
    onionSkinningButton_Off_->setIconSize(QSize(32,32));
    onionSkinningButton_Off_->setFixedSize(buttonSize+20,buttonSize);
    onionSkinningButton_Off_->setAutoExclusive(true);
    onionSkinningButton_Off_->setCheckable(true);
    onionSkinningButton_Off_->setChecked(true);

    onionSkinningButton_On_ = new QPushButton();
    onionSkinningButton_On_->setToolTip("Onion skinning on");
    onionSkinningButton_On_->setIcon(QIcon(":images/onion-skinning-on.png"));
    onionSkinningButton_On_->setIconSize(QSize(32,32));
    onionSkinningButton_On_->setFixedSize(buttonSize+20,buttonSize);
    onionSkinningButton_On_->setAutoExclusive(true);
    onionSkinningButton_On_->setCheckable(true);
    onionSkinningButton_On_->setChecked(false);

    onionSkinningLayoutLeftColumn->addWidget(onionSkinningButton_Off_);
    onionSkinningLayoutLeftColumn->addWidget(onionSkinningButton_On_);
    onionSkinningLayoutLeftColumn->addStretch();

    numOnionSkinsBefore_ = new QSpinBox();
    numOnionSkinsBefore_->setRange(0,100);
    onionSkinningLayoutRightColumn->addRow(tr("Num skins before"), numOnionSkinsBefore_);

    numOnionSkinsAfter_ = new QSpinBox();
    numOnionSkinsAfter_->setRange(0,100);
    onionSkinningLayoutRightColumn->addRow(tr("Num skins after"), numOnionSkinsAfter_);

    areOnionSkinsPickable_ = new QCheckBox();
    onionSkinningLayoutRightColumn->addRow(tr("Are skins pickable"), areOnionSkinsPickable_);

    onionSkinsTimeOffset_ = new QDoubleSpinBox();
    onionSkinsTimeOffset_->setRange(-100,100);
    onionSkinsTimeOffset_->setDecimals(2);
    onionSkinningLayoutRightColumn2->addRow(tr("Skins frame offset"), onionSkinsTimeOffset_);

    onionSkinsXOffset_ = new QDoubleSpinBox();
    onionSkinsXOffset_->setRange(-10000,10000);
    onionSkinsXOffset_->setDecimals(2);
    onionSkinningLayoutRightColumn2->addRow(tr("Skin X offset"), onionSkinsXOffset_);

    onionSkinsYOffset_ = new QDoubleSpinBox();
    onionSkinsYOffset_->setRange(-10000,10000);
    onionSkinsYOffset_->setDecimals(2);
    onionSkinningLayoutRightColumn2->addRow(tr("Skin Y offset"), onionSkinsYOffset_);

    QWidget * onionSkinningWidget = new QWidget();
    onionSkinningLayout->addLayout(onionSkinningLayoutLeftColumn);
    onionSkinningLayout->addLayout(onionSkinningLayoutRightColumn);
    onionSkinningLayout->addLayout(onionSkinningLayoutRightColumn2);
    onionSkinningWidget->setLayout(onionSkinningLayout);

    QMenu * onionSkinningMenu = new QMenu();
    QWidgetAction * onionSkinningWidgetAction = new QWidgetAction(this);
    onionSkinningWidgetAction->setDefaultWidget(onionSkinningWidget);
    onionSkinningMenu->addAction(onionSkinningWidgetAction);


    // Button to show/hide the display mode menu

    onionSkinningButton_ = new QPushButton();
    onionSkinningButton_->setToolTip("Onion skinning settings");
    onionSkinningButton_->setIcon(QIcon(":images/display-mode-normal.png"));
    onionSkinningButton_->setIconSize(QSize(32,32));
    onionSkinningButton_->setFixedSize(buttonSize+20,buttonSize);
    onionSkinningButton_->setMenu(onionSkinningMenu);


    // Layouts and widgets for grouping

    QHBoxLayout * hlayout = new QHBoxLayout();
    hlayout->setMargin(0);
    hlayout->setSpacing(0);
    hlayout->addLayout(frameZoomLayout);
    hlayout->addWidget(displayModeButton_);
    hlayout->addWidget(onionSkinningButton_);

    containerWidget = new QWidget();
    containerWidget->setLayout(hlayout);
    containerWidget->setVisible(true);

    QHBoxLayout * hlayoutfull = new QHBoxLayout();
    hlayoutfull->setMargin(0);
    hlayoutfull->setSpacing(0);
    hlayoutfull->addWidget(showHideSettingsButton_);
    hlayoutfull->addWidget(containerWidget);

    setLayout(hlayoutfull);


    // ----------------- Backend/Gui connections -----------------

    updateWidgetFromSettings(); // Might not be an exact match due to widget min/max values
    updateSettingsFromWidget(); // Make sure its an exact match

    /* THE FOLLOWING ARE NOT IMPLEMENTED OR DEEMED IRRELEVANT
    connect(screenRelative_, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(onionSkinsTransparencyRatio_, SIGNAL(valueChanged(double)), this, SLOT(updateSettingsFromWidget()));
    */

    // Widget -> Settings connection
    connect(zoomSpinBox_, SIGNAL(valueChanged(int)), this, SLOT(processZoomValueChangedSignal(int)));
    connect(frameLineEdit_, SIGNAL(returnPressed()), this,  SLOT(updateSettingsFromWidget()));
    connect(goToPreviousFrameButton, SIGNAL(clicked(bool)), this, SLOT(decrFrame()));
    connect(goToNextFrameButton, SIGNAL(clicked(bool)), this, SLOT(incrFrame()));

    connect(displayModeButton_Normal_, SIGNAL(clicked(bool)), this, SLOT(updateSettingsFromWidget()));
    connect(displayModeButton_NormalOutline_, SIGNAL(clicked(bool)), this, SLOT(updateSettingsFromWidget()));
    connect(displayModeButton_Outline_, SIGNAL(clicked(bool)), this, SLOT(updateSettingsFromWidget()));
    connect(onionSkinningButton_Off_, SIGNAL(clicked(bool)), this, SLOT(updateSettingsFromWidget()));
    connect(onionSkinningButton_On_, SIGNAL(clicked(bool)), this, SLOT(updateSettingsFromWidget()));

    connect(vertexTopologySize_, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(edgeTopologyWidth_, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(drawTopologyFaces_, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidget()));

    connect(areOnionSkinsPickable_, SIGNAL(stateChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(numOnionSkinsBefore_, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(numOnionSkinsAfter_, SIGNAL(valueChanged(int)), this, SLOT(updateSettingsFromWidget()));
    connect(onionSkinsTimeOffset_, SIGNAL(valueChanged(double)), this, SLOT(updateSettingsFromWidget()));
    connect(onionSkinsXOffset_, SIGNAL(valueChanged(double)), this, SLOT(updateSettingsFromWidget()));
    connect(onionSkinsYOffset_, SIGNAL(valueChanged(double)), this, SLOT(updateSettingsFromWidget()));


    // Settings -> Widget connection
    //  -> must be done manually by owner of ViewSettings and ViewSettingsWidget


    // ----------------- Set position and size -----------------

    setGeometry(10,10,100,100); // Move to (10,10) and resize arbitrarily to (100,100)
    setFixedSize(sizeHint());   // Resize to optimal size

#endif
}

ViewSettingsWidget::~ViewSettingsWidget()
{
}

void ViewSettingsWidget::setActive(bool isActive)
{
    if(isActive)
    {
        showHideSettingsButton_->setIcon(QIcon(":images/view-settings-active.png"));
    }
    else
    {
        showHideSettingsButton_->setIcon(QIcon(":images/view-settings.png"));
    }
}

void ViewSettingsWidget::toggleVisible(bool checked)
{
    containerWidget->setVisible(checked);
    setFixedSize(sizeHint());
}

void ViewSettingsWidget::processZoomValueChangedSignal(int n)
{
    // Only set value from spinbox when user actual change value from
    // spinbox, not when setValue() is called programmatically. We do
    // this by manually setting
    //     ignoreZoomValueChangedSignal_ = true;
    // before calling setValue()
    if(!ignoreZoomValueChangedSignal_)
    {
        zoomValue_ = 0.01 * n;
        updateSettingsFromWidget();
    }
}


void ViewSettingsWidget::updateWidgetFromSettings()
{
    /* THE FOLLOWING ARE NOT IMPLEMENTED OR DEEMED IRRELEVANT
    screenRelative_->setChecked(viewSettings_.screenRelative());
    onionSkinsTransparencyRatio_->setValue(viewSettings_.onionSkinsTransparencyRatio());
*/

    vertexTopologySize_->setValue(viewSettings_.vertexTopologySize());
    edgeTopologyWidth_->setValue(viewSettings_.edgeTopologyWidth());
    drawTopologyFaces_->setChecked(viewSettings_.drawTopologyFaces());

    areOnionSkinsPickable_->setChecked(viewSettings_.areOnionSkinsPickable());
    numOnionSkinsBefore_->setValue(viewSettings_.numOnionSkinsBefore());
    numOnionSkinsAfter_->setValue(viewSettings_.numOnionSkinsAfter());
    onionSkinsTimeOffset_->setValue(viewSettings_.onionSkinsTimeOffset().floatTime());
    onionSkinsXOffset_->setValue(viewSettings_.onionSkinsXOffset());
    onionSkinsYOffset_->setValue(viewSettings_.onionSkinsYOffset());

    zoomValue_ = viewSettings_.zoom();
    ignoreZoomValueChangedSignal_ = true;
    zoomSpinBox_->setValue(100*viewSettings_.zoom());
    ignoreZoomValueChangedSignal_ = false;

    frameLineEdit_->setText(QString().setNum(viewSettings_.time().frame()));

    switch(viewSettings_.displayMode())
    {
    case ViewSettings::ILLUSTRATION:
        displayModeButton_Normal_->setChecked(true);
        displayModeButton_->setIcon(QIcon(":images/display-mode-normal.png"));
        break;
    case ViewSettings::ILLUSTRATION_OUTLINE:
        displayModeButton_NormalOutline_->setChecked(true);
        displayModeButton_->setIcon(QIcon(":images/display-mode-normal-outline.png"));
        break;
    case ViewSettings::OUTLINE:
        displayModeButton_Outline_->setChecked(true);
        displayModeButton_->setIcon(QIcon(":images/display-mode-outline.png"));
        break;
    }

    switch(viewSettings_.onionSkinningIsEnabled())
    {
    case false:
        onionSkinningButton_Off_->setChecked(true);
        onionSkinningButton_->setIcon(QIcon(":images/onion-skinning-off.png"));
        break;
    case true:
        onionSkinningButton_On_->setChecked(true);
        onionSkinningButton_->setIcon(QIcon(":images/onion-skinning-on.png"));
        break;
    }
}

void ViewSettingsWidget::updateSettingsFromWidget()
{
    updateSettingsFromWidgetSilent();
    emit changed();
}

int ViewSettingsWidget::getFrame_()
{
    bool ok;
    int frame = frameLineEdit_->text().toInt(&ok);
    if(ok)
    {
        return frame;
    }
    else
    {
        return viewSettings_.time().frame();
    }
}

void ViewSettingsWidget::incrFrame()
{
    int frame = getFrame_();
    frame++;
    frameLineEdit_->setText(QString().setNum(frame));
    updateSettingsFromWidget();
}

void ViewSettingsWidget::decrFrame()
{
    int frame = getFrame_();
    frame--;
    frameLineEdit_->setText(QString().setNum(frame));
    updateSettingsFromWidget();
}

void ViewSettingsWidget::updateSettingsFromWidgetSilent()
{
    /* THE FOLLOWING ARE NOT IMPLEMENTED OR DEEMED IRRELEVANT
    viewSettings_.setScreenRelative(screenRelative_->isChecked());
    viewSettings_.setOnionSkinsTransparencyRatio(onionSkinsTransparencyRatio_->value());
    */

    viewSettings_.setVertexTopologySize(vertexTopologySize_->value());
    viewSettings_.setEdgeTopologyWidth(edgeTopologyWidth_->value());
    viewSettings_.setDrawTopologyFaces(drawTopologyFaces_->isChecked());

    viewSettings_.setAreOnionSkinsPickable(areOnionSkinsPickable_->isChecked());
    viewSettings_.setNumOnionSkinsBefore(numOnionSkinsBefore_->value());
    viewSettings_.setNumOnionSkinsAfter(numOnionSkinsAfter_->value());
    viewSettings_.setOnionSkinsTimeOffset(onionSkinsTimeOffset_->value());
    viewSettings_.setOnionSkinsXOffset(onionSkinsXOffset_->value());
    viewSettings_.setOnionSkinsYOffset(onionSkinsYOffset_->value());

    viewSettings_.setZoom(zoomValue_);
    QString textFrame = frameLineEdit_->text();
    bool ok;
    int frame = textFrame.toInt(&ok);
    if(ok)
    {
        viewSettings_.setTime(Time(frame));
    }
    else
    {
        frameLineEdit_->setText(QString().setNum(viewSettings_.time().frame()));
    }

    if(displayModeButton_Normal_->isChecked())
    {
        viewSettings_.setDisplayMode(ViewSettings::ILLUSTRATION);
        displayModeButton_->setIcon(QIcon(":images/display-mode-normal.png"));
    }
    else if(displayModeButton_NormalOutline_->isChecked())
    {
        viewSettings_.setDisplayMode(ViewSettings::ILLUSTRATION_OUTLINE);
        displayModeButton_->setIcon(QIcon(":images/display-mode-normal-outline.png"));

    }
    else if(displayModeButton_Outline_->isChecked())
    {
        viewSettings_.setDisplayMode(ViewSettings::OUTLINE);
        displayModeButton_->setIcon(QIcon(":images/display-mode-outline.png"));
    }

    if(onionSkinningButton_Off_->isChecked())
    {
        viewSettings_.setOnionSkinningIsEnabled(false);
        onionSkinningButton_->setIcon(QIcon(":images/onion-skinning-off.png"));
    }
    else if(onionSkinningButton_On_->isChecked())
    {
        viewSettings_.setOnionSkinningIsEnabled(true);
        onionSkinningButton_->setIcon(QIcon(":images/onion-skinning-on.png"));
    }

}

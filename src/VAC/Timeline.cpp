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

#include "Timeline.h"

#include <QSpinBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QPainter>
#include <QTimer>
#include <QElapsedTimer>
#include <QDialogButtonBox>

#include <QMouseEvent>
#include <QtDebug>

#include "Scene.h"
#include "View.h"
#include "Global.h"

#include "VectorAnimationComplex/VAC.h"
#include "VectorAnimationComplex/Cell.h"
#include "VectorAnimationComplex/KeyCell.h"
#include "VectorAnimationComplex/InbetweenCell.h"

#include "XmlStreamReader.h"
#include "XmlStreamWriter.h"

using VectorAnimationComplex::VAC;
using VectorAnimationComplex::Cell;
using VectorAnimationComplex::KeyCell;
using VectorAnimationComplex::InbetweenCell;
using VectorAnimationComplex::CellSet;
using VectorAnimationComplex::KeyCellSet;
using VectorAnimationComplex::InbetweenCellSet;

Timeline_HBar::Timeline_HBar(Timeline * w) :
    QWidget(w),
    w_(w),
    isScrolling_(false),
    hasHighlightedFrame_(false)
{
    // colors
    colors_ << Qt::red << Qt::blue;

    // set the recommended size
    setMinimumSize(500, 20);
    setMaximumSize(5000, 20);

    // set the background color
    setAutoFillBackground(true);
    QPalette palette(Qt::white, Qt::white);
    setPalette(palette);

    // track the mouse for cell highlighting
    setMouseTracking(true);
}

void Timeline_HBar::paintEvent (QPaintEvent * /*event*/)
{
    // compute frame range to display
    w_->firstVisibleFrame_ = (w_->totalPixelOffset_ / 10);
    w_->lastVisibleFrame_ = ((w_->totalPixelOffset_ + width())/ 10);

    // initialize painter
    QPainter painter(this);

    // draw grey background for cell out of the playing window
    painter.setBrush(QColor(200,200,200));
    painter.setPen(Qt::NoPen);
    if(w_->firstVisibleFrame_<=w_->firstFrame())
        painter.drawRect(
            0, 1,
            10*(w_->firstFrame())-w_->totalPixelOffset_,height()-2);
    if(w_->lastVisibleFrame_>=w_->lastFrame())
        painter.drawRect(
            10*(w_->lastFrame()+1)-w_->totalPixelOffset_, 1,
            width()-1-10*(w_->lastFrame() - w_->firstVisibleFrame_),
            height()-2);

    // highlighted frame
    //if(w_->isInOneTimeMode_ || w_->activeTime() == 1)
        painter.setBrush(QColor(255,150,150));
    //else
    //    painter.setBrush(QColor(150,150,255));
    painter.setPen(Qt::NoPen);
    if(hasHighlightedFrame_/* && highlightedFrame_!=w_->currentFrame1_ && highlightedFrame_!=w_->currentFrame2_*/)
        painter.drawRect(10*(highlightedFrame_) - w_->totalPixelOffset_ + 1, 1, 9, height()-2);



    // current frames
    painter.setBrush(Qt::red);
    painter.setPen(Qt::NoPen);
    for(View * view: qAsConst(w_->views_))
    {
        painter.drawRect(10*(view->activeTime().floatTime()) - w_->totalPixelOffset_ + 1, 1, 9, height()-2);
    }
    painter.setBrush(QColor(200,0,0));
    painter.drawRect(10*(global()->activeTime().floatTime()) - w_->totalPixelOffset_ + 1, 1, 9, height()-2);

    /*
    painter.drawRect(10*(w_->currentFrame1_) - w_->totalPixelOffset_ + 1, 1, 9, height()-2);

    // current frame 2
    if(!w_->isInOneTimeMode_)
    {
        painter.setBrush(Qt::blue);
        painter.setPen(Qt::NoPen);
        painter.drawRect(10*(w_->currentFrame2_) - w_->totalPixelOffset_ + 1, 1, 9, height()-2);
    }
    */

    // vertical bar between frames
    painter.setPen(QColor(150,150,200));
    for(int i=w_->firstVisibleFrame_; i<=w_->lastVisibleFrame_; i++)
        painter.drawLine(10*i - w_->totalPixelOffset_, 1, 10*i - w_->totalPixelOffset_, height()-2);
    
    
    // border
    painter.setPen(QColor(50,50,50));
    painter.drawLine(0, 0, width() - 1, 0);
    painter.drawLine(0, height()-1, width() - 1, height()-1);
    painter.drawLine(0, 1, 0, height()-2);
    painter.drawLine(width()-1, 1, width()-1, height()-2);

    // Get cells
    VAC * vac = w_->scene_->activeVAC();
    if (!vac) {
        return;
    }
    CellSet cells = vac->cells();
    KeyCellSet keyCells = cells;
    InbetweenCellSet inbetweenCells = cells;
    CellSet selectedCells = vac->selectedCells();
    KeyCellSet selectedKeyCells = selectedCells;
    InbetweenCellSet selectedInbetweenCells = selectedCells;

    // Draw inbetween cells
    painter.setPen(QColor(0,0,0));
    painter.setBrush(QColor(0,0,0));
    for(InbetweenCell * inbetweenCell: inbetweenCells)
    {
        double t1 = inbetweenCell->beforeTime().floatTime();
        double t2 = inbetweenCell->afterTime().floatTime();
        painter.drawRect(10*t1 - w_->totalPixelOffset_ + 5, 4,
                         10*(t2-t1), 2);
        //painter.drawLine(10*t1 - w_->totalPixelOffset_ + 5, 5,
        //                 10*t2 - w_->totalPixelOffset_ + 5, 5);
    }
    painter.setBrush(QColor(255,0,0));
    for(InbetweenCell * inbetweenCell: selectedInbetweenCells)
    {
        double t1 = inbetweenCell->beforeTime().floatTime();
        double t2 = inbetweenCell->afterTime().floatTime();
        painter.drawRect(10*t1 - w_->totalPixelOffset_ + 5, 4,
                         10*(t2-t1), 2);
        //painter.drawLine(10*t1 - w_->totalPixelOffset_ + 5, 5,
        //                 10*t2 - w_->totalPixelOffset_ + 5, 5);
    }

    // Draw key cells
    painter.setPen(QColor(0,0,0));
    painter.setBrush(QColor(0,0,0));
    for(KeyCell * keyCell: keyCells)
    {
        double t = keyCell->time().floatTime();
        painter.drawEllipse(10*t - w_->totalPixelOffset_ + 2, 2, 6, 6);
    }
    painter.setBrush(QColor(255,0,0));
    for(KeyCell * keyCell: selectedKeyCells)
    {
        double t = keyCell->time().floatTime();
        painter.drawEllipse(10*t - w_->totalPixelOffset_ + 2, 2, 6, 6);
    }



    // Selection info
    /*
    if(w_->selectionType_ == 1)
    {
        painter.setPen(QColor(0,0,0));
        painter.setBrush(QColor(0,0,0));

        // draw rect at current time
        painter.drawRect(10*(w_->t_) - w_->totalPixelOffset_ + 1, 1, 8, 8);
    }

    if(w_->selectionType_ == 1 || w_->selectionType_ == 2)
    {
        painter.setPen(QColor(0,0,0));
        painter.setBrush(QColor(0,0,0));

        // draw disc at t1 and t2
        painter.drawEllipse(10*(w_->t1_) - w_->totalPixelOffset_ + 2, 2, 6, 6);
        painter.drawEllipse(10*(w_->t2_) - w_->totalPixelOffset_ + 2, 2, 6, 6);

        // link disks
        painter.drawLine(10*(w_->t1_) - w_->totalPixelOffset_ + 5, 5,
                         10*(w_->t2_) - w_->totalPixelOffset_ + 5, 5);

    }
    */

}

void Timeline_HBar::mousePressEvent (QMouseEvent * event)
{
    // Pan the timeline
    if(event->button() == Qt::MidButton)
    {
        hasHighlightedFrame_ = false;
        scrollingInitialX_ = event->x();
        scrollingInitialOffset_ = w_->totalPixelOffset_;
        isScrolling_ = true;
        setCursor(QCursor(Qt::ClosedHandCursor));
    }

    // Select time
    else if(!isScrolling_ &&
          event->button() == Qt::LeftButton &&
          hasHighlightedFrame_ /*&&
          highlightedFrame_!=w_->currentFrame_()*/)
    {
        w_->goToFrame(global()->activeView(), highlightedFrame_);
    }

    // Temporal Drag and drop
    else if(!isScrolling_ &&
          event->button() == Qt::RightButton &&
          hasHighlightedFrame_)
    {
        setCursor(QCursor(Qt::ClosedHandCursor));
        VectorAnimationComplex::VAC * vac = w_->scene_->activeVAC();
        if (vac)
        {
             vac->prepareTemporalDragAndDrop(Time(highlightedFrame_));
        }
    }


    /*
    else if(!isScrolling_ &&
          !w_->isInOneTimeMode_ &&
          event->button() == Qt::RightButton &&
          hasHighlightedFrame_ &&
          highlightedFrame_!=w_->otherFrame_())
        w_->goToFrame(highlightedFrame_, true);
        */
}

void Timeline_HBar::mouseReleaseEvent (QMouseEvent * event)
{
    if(event->button() == Qt::MidButton)
    {
        isScrolling_ = false;
        if(event->y() >=0 && event->y()<height() && event->x()>0 && event->x()<width())
        {
            hasHighlightedFrame_ = true;
            int pos = (event->x() + w_->totalPixelOffset_);
            if(pos>=0)
                highlightedFrame_ = (event->x() + w_->totalPixelOffset_)/10;
            else
                highlightedFrame_ = (event->x() + w_->totalPixelOffset_)/10-1;
        }
        else
            hasHighlightedFrame_ = false;

        setCursor(QCursor(Qt::ArrowCursor));
    }
    else if(event->button() == Qt::RightButton)
    {
        VectorAnimationComplex::VAC * vac = w_->scene_->activeVAC();
        if (vac)
        {
            vac->completeTemporalDragAndDrop();
            setCursor(QCursor(Qt::ArrowCursor));
        }
    }
    repaint();
}

void Timeline_HBar::mouseMoveEvent (QMouseEvent * event)
{
    if(isScrolling_)
    {
        w_->totalPixelOffset_ = scrollingInitialOffset_ -
            event->x() + scrollingInitialX_ ;
    }
    else
    {
        hasHighlightedFrame_ = true; 
        int pos = (event->x() + w_->totalPixelOffset_);
        if(pos>=0)
            highlightedFrame_ = (event->x() + w_->totalPixelOffset_)/10;
        else
            highlightedFrame_ = (event->x() + w_->totalPixelOffset_)/10-1;
        
        // Select time
        if(event->buttons() & Qt::LeftButton &&
           hasHighlightedFrame_ )
        {
            w_->goToFrame(global()->activeView(), highlightedFrame_);
        }

        // Temporal Drag and drop
        else if(event->buttons() & Qt::RightButton &&
                hasHighlightedFrame_ )
        {
            VectorAnimationComplex::VAC * vac = w_->scene_->activeVAC();
            if (vac)
            {
                vac->performTemporalDragAndDrop(Time(highlightedFrame_));
            }
        }
    }
    
    repaint();
}

void Timeline_HBar::leaveEvent (QEvent * /*event*/)
{
    hasHighlightedFrame_ = false;
    repaint();
}


PlaybackSettings::PlaybackSettings()
{
    setDefaultValues();
}

void PlaybackSettings::setDefaultValues()
{
    setFirstFrame(0);
    setLastFrame(47);
    setFps(24) ;
    setPlayMode(NORMAL);
    setSubframeInbetweening(false);
}

QString PlaybackSettings::playModeToString(PlayMode mode)
{
    switch(mode)
    {
    case NORMAL:
        return "normal";
    case LOOP:
        return "loop";
    case BOUNCE:
        return "bounce";
    }

    return "normal";
}

PlaybackSettings::PlayMode PlaybackSettings::stringToPlayMode(const QString & str)
{
    if(str == "normal")
        return NORMAL;
    else if(str == "loop")
        return LOOP;
    else if(str == "bounce")
        return BOUNCE;
    else
        return NORMAL;
}

int PlaybackSettings::firstFrame() const { return firstFrame_; }
int PlaybackSettings::lastFrame() const { return lastFrame_; }
int PlaybackSettings::fps() const { return fps_; }
PlaybackSettings::PlayMode PlaybackSettings::playMode() const { return playMode_; }
bool PlaybackSettings::subframeInbetweening() const { return subframeInbetweening_; }

void PlaybackSettings::setFirstFrame(int f) { firstFrame_ = f; }
void PlaybackSettings::setLastFrame(int f) { lastFrame_ = f; }
void PlaybackSettings::setFps(int n)  { fps_ = n; }
void PlaybackSettings::setPlayMode(PlayMode mode) { playMode_ = mode; }
void PlaybackSettings::setSubframeInbetweening(bool b) { subframeInbetweening_ = b; }

void PlaybackSettings::read(XmlStreamReader & xml)
{
    setDefaultValues();

    if(xml.attributes().hasAttribute("framerange"))
    {
        QString stringRange = xml.attributes().value("framerange").toString();
        QStringList list = stringRange.split(" ");
        setFirstFrame(list[0].toInt());
        setLastFrame(list[1].toInt());
    }
    if(xml.attributes().hasAttribute("fps"))
        setFps(xml.attributes().value("fps").toInt());
    if(xml.attributes().hasAttribute("playmode"))
        setPlayMode(stringToPlayMode(xml.attributes().value("playmode").toString()));
    if(xml.attributes().hasAttribute("subframeinbetweening"))
        setSubframeInbetweening((xml.attributes().value("subframeinbetweening") == "on") ? true : false);

    xml.skipCurrentElement();
}

void PlaybackSettings::write(XmlStreamWriter & xml) const
{
    xml.writeAttribute("framerange", QString().setNum(firstFrame()) + " " + QString().setNum(lastFrame()));
    xml.writeAttribute("fps", QString().setNum(fps()));
    xml.writeAttribute("subframeinbetweening", subframeInbetweening() ? "on" : "off");
    xml.writeAttribute("playmode", playModeToString(playMode()));
}



PlaybackSettingsDialog::PlaybackSettingsDialog(const PlaybackSettings & settings) :
    QDialog()
{
    // Window title
    setWindowTitle(tr("Playback Settings"));

    // Create widgets holding settings values
    //   FPS
    fpsSpinBox_ = new QSpinBox();
    fpsSpinBox_->setRange(1,200);
    //   Playback Mode
    playModeSpinBox_ = new QComboBox();
    playModeSpinBox_->addItem("Normal");
    playModeSpinBox_->addItem("Loop");
    playModeSpinBox_->addItem("Bounce");
    //   Subframe Inbetweening
    subframeCheckBox_ = new QCheckBox();

    // Init values of widgets
    setPlaybackSettings(settings);

    // Organize widgets into a form layout
    QFormLayout * formLayout = new QFormLayout();
    formLayout->addRow(tr("FPS"), fpsSpinBox_);
    formLayout->addRow(tr("Play Mode"), playModeSpinBox_);
    formLayout->addRow(tr("Subframe Inbetweening"), subframeCheckBox_);

    // Create OK/Cancel buttons
    QDialogButtonBox * buttonBox = new QDialogButtonBox(
                QDialogButtonBox::Ok |
                QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    // Create and set dialog layout
    QVBoxLayout * layout = new QVBoxLayout();
    layout->addLayout(formLayout);
    layout->addStretch();
    layout->addWidget(buttonBox);
    setLayout(layout);
}

PlaybackSettings PlaybackSettingsDialog::playbackSettings() const
{
    settings_.setFps(fpsSpinBox_->value());
    settings_.setSubframeInbetweening(subframeCheckBox_->isChecked());
    settings_.setPlayMode(static_cast<PlaybackSettings::PlayMode>(playModeSpinBox_->currentIndex()));

    return settings_;
}

void PlaybackSettingsDialog::setPlaybackSettings(const PlaybackSettings & settings)
{
    settings_ = settings;

    fpsSpinBox_->setValue(settings_.fps());
    subframeCheckBox_->setChecked(settings_.subframeInbetweening());
    playModeSpinBox_->setCurrentIndex(static_cast<int>(settings_.playMode()));
}

void Timeline::read(XmlStreamReader & xml)
{
    settings_.read(xml);

    setFirstFrame(settings_.firstFrame());
    setLastFrame(settings_.lastFrame());
    setFps(settings_.fps());
}

void Timeline::write(XmlStreamWriter & xml) const
{
    settings_.write(xml);
}

namespace
{

QPushButton * makeButton_(const QString & iconPath, QAction * action)
{
    QPushButton * button = new QPushButton(QIcon(iconPath), "");
#ifdef Q_OS_MAC
    button->setMaximumWidth(50);
#else
    button->setMaximumSize(32,32);
#endif
    button->setToolTip(action->toolTip());
    button->setStatusTip(action->statusTip());
    QObject::connect(button, SIGNAL(clicked()), action, SLOT(trigger()));
    return button;
}

}

Timeline::Timeline(VPaint::Scene *scene, QWidget *parent) :
    QWidget(parent),
    scene_(scene)
{
    // initialisations
    totalPixelOffset_ = 0;
    selectionType_ = 0;

    // Horizontal bar (must be first cause some setValue() call hbar_->update())
    hbar_ = new Timeline_HBar(this);

    // Open settings
    QPushButton * settingsButton = new QPushButton(tr("Settings"));
#ifdef Q_OS_MAC
    settingsButton->setMaximumWidth(80);
#else
    settingsButton->setMaximumSize(64,32);
#endif
    connect(settingsButton, SIGNAL(clicked()),
          this, SLOT(openPlaybackSettingsDialog()));

    // ----- Create actions -----

    actionGoToFirstFrame_ = new QAction(tr("Go to first frame"), this);
    actionGoToFirstFrame_->setStatusTip(tr("Set frame of active view to be the first frame in playback range."));
    actionGoToFirstFrame_->setToolTip(QString(ACTION_MODIFIER_NAME_SHORT).toUpper() + tr(" + Left"));
    actionGoToFirstFrame_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Left));
    actionGoToFirstFrame_->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionGoToFirstFrame_, SIGNAL(triggered()), this, SLOT(goToFirstFrame()));

    actionGoToPreviousFrame_ = new QAction(tr("Go to previous frame"), this);
    actionGoToPreviousFrame_->setStatusTip(tr("Set frame of active view to be the previous frame."));
    actionGoToPreviousFrame_->setToolTip(tr("Left"));
    actionGoToPreviousFrame_->setShortcut(QKeySequence(Qt::Key_Left));
    actionGoToPreviousFrame_->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionGoToPreviousFrame_, SIGNAL(triggered()), this, SLOT(goToPreviousFrame()));

    actionPlayPause_ = new QAction(tr("Play/Pause"), this);
    actionPlayPause_->setStatusTip(tr("Toggle between play and pause"));
    actionPlayPause_->setToolTip(tr("Space"));
    actionPlayPause_->setShortcut(QKeySequence(Qt::Key_Space));
    actionPlayPause_->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionPlayPause_, SIGNAL(triggered()), this, SLOT(playPause()));

    actionGoToNextFrame_ = new QAction(tr("Go to next frame"), this);
    actionGoToNextFrame_->setStatusTip(tr("Set frame of active view to be the next frame."));
    actionGoToNextFrame_->setToolTip(tr("Right"));
    actionGoToNextFrame_->setShortcut(QKeySequence(Qt::Key_Right));
    actionGoToNextFrame_->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionGoToNextFrame_, SIGNAL(triggered()), this, SLOT(goToNextFrame()));

    actionGoToLastFrame_ = new QAction(tr("Go to last frame"), this);
    actionGoToLastFrame_->setStatusTip(tr("Set frame of active view to be the last frame in playback range."));
    actionGoToLastFrame_->setToolTip(QString(ACTION_MODIFIER_NAME_SHORT).toUpper() + tr(" + Right"));
    actionGoToLastFrame_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Right));
    actionGoToLastFrame_->setShortcutContext(Qt::WindowShortcut);
    connect(actionGoToLastFrame_, SIGNAL(triggered()), this, SLOT(goToLastFrame()));

    // ----- Create buttons -----

    // Go to first frame
    firstFrameButton_ = makeButton_(":/images/go-previous.png", actionGoToFirstFrame_);

    // Go to previous frame
    previousFrameButton_ = makeButton_(":/images/go-first-view.png", actionGoToPreviousFrame_);

    // Play/pause
    playPauseButton_ = makeButton_(":/images/go-play.png", actionPlayPause_);

    // Go to next frame
    nextFrameButton_ = makeButton_(":/images/go-last-view.png", actionGoToNextFrame_);

    // Go to last frame
    lastFrameButton_ = makeButton_(":/images/go-next.png", actionGoToLastFrame_);

    // Set first frame
    firstFrameSpinBox_ = new QSpinBox();
#ifdef Q_OS_MAC
    firstFrameSpinBox_->setMaximumWidth(48);
#else
    firstFrameSpinBox_->setMaximumSize(48,32);
#endif
    firstFrameSpinBox_->setMinimum(-100000); // 100.000 frames = about 1h at 24fps
    firstFrameSpinBox_->setMaximum(100000); // 100.000 frames = about 1h at 24fps
    setFirstFrame(0);
    connect(firstFrameSpinBox_, SIGNAL(valueChanged(int)), this, SLOT(setFirstFrame(int)));

    // Set last Frame
    lastFrameSpinBox_ = new QSpinBox();
#ifdef Q_OS_MAC
    lastFrameSpinBox_->setMaximumWidth(48);
#else
    lastFrameSpinBox_->setMaximumSize(48,32);
#endif
    lastFrameSpinBox_->setMinimum(-100000); // 100.000 frames = about 1h at 24fps
    lastFrameSpinBox_->setMaximum(100000); // 100.000 frames = about 1h at 24fps
    setLastFrame(47);
    connect(lastFrameSpinBox_, SIGNAL(valueChanged(int)), this, SLOT(setLastFrame(int)));

    // Set FPS
    timer_ = new QTimer();
    setFps(24);
    connect(timer_, SIGNAL(timeout()), this, SLOT(timerTimeout()));

    // Global layout
    QHBoxLayout * layout = new QHBoxLayout();
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->setContentsMargins(0, 8, 5, 0);
    layout->addWidget(settingsButton);
    layout->addSpacing(5);
    layout->addWidget(firstFrameButton_);
    layout->addWidget(previousFrameButton_);
    layout->addWidget(playPauseButton_);
    layout->addWidget(nextFrameButton_);
    layout->addWidget(lastFrameButton_);
    layout->addSpacing(5);
    layout->addWidget(firstFrameSpinBox_);
    layout->addSpacing(5);
    layout->addWidget(hbar_);
    layout->addSpacing(5);
    layout->addWidget(lastFrameSpinBox_);
    setLayout(layout);
}

Timeline::~Timeline()
{
    delete timer_;
}

QAction * Timeline::actionGoToFirstFrame() const
{
    return actionGoToFirstFrame_;
}

QAction * Timeline::actionGoToPreviousFrame() const
{
    return actionGoToPreviousFrame_;
}

QAction * Timeline::actionPlayPause() const
{
    return actionPlayPause_;
}

QAction * Timeline::actionGoToNextFrame() const
{
    return actionGoToNextFrame_;
}

QAction * Timeline::actionGoToLastFrame() const
{
    return actionGoToLastFrame_;
}


void Timeline::setSelectionType(int type)
{
    selectionType_ = type;
    update();
}

void Timeline::setT(double t)
{
    t_ = t;
    update();
}

void Timeline::setT1(double t1)
{
    t1_ = t1;
    update();
}

void Timeline::setT2(double t2)
{
    t2_ = t2;
    update();
}

void Timeline::paintEvent(QPaintEvent * event)
{
    hbar_->update();
    QWidget::paintEvent(event);
}


int Timeline::firstFrame() const
{
    return settings_.firstFrame();
}

int Timeline::lastFrame() const
{
    return settings_.lastFrame();
}

int Timeline::fps() const
{
    return settings_.fps();
}

bool Timeline::subframeInbetweening() const
{
    return settings_.subframeInbetweening();
}

PlaybackSettings::PlayMode Timeline::playMode() const
{
    return settings_.playMode();
}

int Timeline::firstVisibleFrame() const
{
    return firstVisibleFrame_;
}

int Timeline::lastVisibleFrame() const
{
    return lastVisibleFrame_;
}

void Timeline::play()
{
    if(playMode() != PlaybackSettings::BOUNCE)
        playingDirection_ = true;

    playedViews_.clear();
    View * view = global()->activeView();
    if(view)
    {
        playedViews_ << global()->activeView();
        for(View * view: qAsConst(playedViews_))
            view->disablePicking();
        elapsedTimer_.start();
        timer_->start();
        playPauseButton_->setIcon(QIcon(":/images/go-pause.png"));
    }
}

void Timeline::pause()
{
    timer_->stop();
    for(View * view: qAsConst(playedViews_))
        view->enablePicking();
    roundPlayedViews();
    playPauseButton_->setIcon(QIcon(":/images/go-play.png"));
}

void Timeline::playPause()
{
    if(isPlaying())
        pause();
    else
        play();
}


void Timeline::roundPlayedViews()
{
    for(View * view: qAsConst(playedViews_))
    {
        Time t = view->activeTime();
        double floatFrame = t.floatTime();
        int intFrame = std::floor(floatFrame+0.5);
        goToFrame(view, intFrame);
    }
}

void Timeline::openPlaybackSettingsDialog()
{
    PlaybackSettingsDialog * dialog = new PlaybackSettingsDialog(settings_);
    int accepted = dialog->exec();
    if(accepted)
    {
        settings_ = dialog->playbackSettings();
        setFps(fps());
    }
    delete dialog;
}

void Timeline::goToFirstFrame()
{
    goToFirstFrame(global()->activeView());
}

void Timeline::goToFirstFrame(View * view)
{
    goToFrame(view, firstFrame());
}

void Timeline::goToLastFrame()
{
    goToLastFrame(global()->activeView());
}

void Timeline::goToLastFrame(View * view)
{
    goToFrame(view, lastFrame());
}

void Timeline::setFirstFrame(int firstFrame)
{
    if(firstFrame > lastFrame())
    {
        firstFrame = lastFrame();
    }
    if(firstFrameSpinBox_->value() != firstFrame)
    {
        firstFrameSpinBox_->setValue(firstFrame);
        lastFrameSpinBox_->setMinimum(firstFrame);
    }
    settings_.setFirstFrame(firstFrame);
    hbar_->update();
    emit playingWindowChanged();
}

void Timeline::setLastFrame(int lastFrame)
{
    if(lastFrame < firstFrame()) {
        lastFrame = firstFrame();
    }
    if(lastFrameSpinBox_->value() != lastFrame)
    {
        lastFrameSpinBox_->setValue(lastFrame);
        firstFrameSpinBox_->setMaximum(lastFrame);
    }
    settings_.setLastFrame(lastFrame);
    hbar_->update();
    emit playingWindowChanged();
}

void Timeline::setFps(int fps)
{
    if(subframeInbetweening())
    {
        timer_->setInterval(0);
    }
    else
    {
        int msec = 1000 / fps;
        timer_->setInterval(msec);
    }
}

void Timeline::realTimePlayingChanged()
{
    setFps(fps());
}

void Timeline::timerTimeout()
{
    int elapsedMsec = elapsedTimer_.elapsed();
    if(elapsedMsec == 0)
        return;

    elapsedTimer_.restart();

    for(View * view: qAsConst(playedViews_))
    {
        if(isPlaying() && subframeInbetweening())
        {
            double nextFrame = view->activeTime().floatTime();

            if(playingDirection_)
                nextFrame += 0.001 * elapsedMsec * fps();
            else
                nextFrame -= 0.001 * elapsedMsec * fps();

            switch(playMode())
            {
            case PlaybackSettings::NORMAL:
                if(nextFrame > lastFrame())
                   pause();
                else if(nextFrame < firstFrame())
                    goToFrame(view, firstFrame());
                else
                    goToFrame(view, nextFrame);
                break;

            case PlaybackSettings::LOOP:
                if(nextFrame > lastFrame())
                    goToFrame(view, firstFrame());
                else if(nextFrame < firstFrame())
                    goToFrame(view, firstFrame());
                else
                    goToFrame(view, nextFrame);
                break;

            case PlaybackSettings::BOUNCE:
                if(nextFrame > lastFrame())
                {
                    playingDirection_ = false;
                    goToFrame(view, lastFrame());
                }
                else if(nextFrame < firstFrame())
                {
                    playingDirection_ = true;
                    goToFrame(view, firstFrame());
                }
                else
                    goToFrame(view, nextFrame);
                break;
            }
        }
        else
        {
            switch(playMode())
            {
            case PlaybackSettings::NORMAL:
            case PlaybackSettings::LOOP:
                if(playingDirection_)
                    goToNextFrame(view);
                else
                    goToPreviousFrame(view);
                break;

            case PlaybackSettings::BOUNCE:
                if(view->activeTime() >= lastFrame())
                {
                    playingDirection_ = false;
                    goToFrame(view, lastFrame()-1);
                }
                else if(view->activeTime() <= firstFrame())
                {
                    playingDirection_ = true;
                    goToFrame(view, firstFrame()+1);
                }
                else
                {
                    if(playingDirection_)
                        goToNextFrame(view);
                    else
                        goToPreviousFrame(view);
                }
                break;
            }
        }
    }
}

void Timeline::goToNextFrame()
{
    goToNextFrame(global()->activeView());
}

// There are multiple implementations of goToNextFrame and goToPreviousFrame
// See https://github.com/dalboris/vpaint/pull/4#issuecomment-130426290 for more details
// Will likely be configurable through preferences one day
// Implementation 1
void Timeline::goToNextFrame(View * view)
{
    int currentFrame = view->activeTime().floatTime();

    if(isPlaying()) {
        if(currentFrame < firstFrame())
        {
            goToFrame(view, firstFrame());
        }
        else if(currentFrame >= lastFrame())
        {
            if(playMode() == PlaybackSettings::LOOP)
            {
                goToFrame(view, firstFrame());
            }
            else
            {
                pause();
            }
        }
        else
        {
            goToFrame(view, currentFrame+1);
        }
    }
    else
    {
        goToFrame(view, currentFrame+1);
    }
}

// Implementation 2
/*void Timeline::goToNextFrame(View * view)
{
    int currentFrame = view->activeTime().floatTime();

    if(currentFrame < firstFrame())
    {
        goToFrame(view, firstFrame());
    }
    else if(currentFrame >= lastFrame())
    {
        if(playMode() == PlaybackSettings::LOOP)
        {
            goToFrame(view, firstFrame());
        }
        else
        {
            pause();
        }
    }
    else
    {
        goToFrame(view, currentFrame+1);
    }
}*/

// Implemenation 3
/*void Timeline::goToNextFrame(View * view)
{
    int currentFrame = view->activeTime().floatTime();

    if(currentFrame < firstFrame())
    {
        goToFrame(view, firstFrame());
    }
    else if(currentFrame >= lastFrame())
    {
        if(playMode() == PlaybackSettings::LOOP && isPlaying())
        {
            goToFrame(view, firstFrame());
        }
        else
        {
            pause();
        }
    }
    else
    {
        goToFrame(view, currentFrame+1);
    }
}*/

void Timeline::goToPreviousFrame()
{
    goToPreviousFrame(global()->activeView());
}

// See comment above goToNextFrame
// Implementation 1
void Timeline::goToPreviousFrame(View * view)
{
    int currentFrame = view->activeTime().floatTime();

    if(isPlaying()) {
        if(currentFrame > lastFrame())
        {
            goToFrame(view, lastFrame());
        }
        else if(currentFrame <= firstFrame())
        {
            if(playMode() == PlaybackSettings::LOOP)
            {
                goToFrame(view, lastFrame());
            }
            else
            {
                pause();
            }
        }
        else
        {
            goToFrame(view, currentFrame-1);
        }
    }
    else
    {
        goToFrame(view, currentFrame-1);
    }
}

// Implementation 2
/*void Timeline::goToPreviousFrame(View * view)
{
    int currentFrame = view->activeTime().floatTime();

    if(currentFrame > lastFrame())
    {
        goToFrame(view, lastFrame());
    }
    else if(currentFrame <= firstFrame())
    {
        if(playMode() == PlaybackSettings::LOOP)
        {
            goToFrame(view, lastFrame());
        }
        else
        {
            pause();
        }
    }
    else
    {
        goToFrame(view, currentFrame-1);
    }
}*/

// Implemenation 3
/*void Timeline::goToPreviousFrame(View * view)
{
    int currentFrame = view->activeTime().floatTime();

    if(currentFrame > lastFrame())
    {
        goToFrame(view, lastFrame());
    }
    else if(currentFrame <= firstFrame())
    {
        if(playMode() == PlaybackSettings::LOOP && isPlaying())
        {
            goToFrame(view, lastFrame());
        }
        else
        {
            pause();
        }
    }
    else
    {
        goToFrame(view, currentFrame-1);
    }
}*/

void Timeline::goToFrame(View * view, double frame)
{
    view->setActiveTime(Time(frame)); // float time
    hbar_->repaint();
    emit timeChanged();
}

void Timeline::goToFrame(View * view, int frame)
{
    view->setActiveTime(Time(frame)); // exact frame
    hbar_->repaint();
    emit timeChanged();
}

void Timeline::addView(View * view)
{
    views_ << view;
    connect(view, SIGNAL(settingsChanged()), this, SLOT(update()));
    hbar_->update();
}

void Timeline::removeView(View * view)
{
    views_.removeAll(view);
    hbar_->update();
}

bool Timeline::isPlaying() const
{
    return timer_->isActive();
}

QSet<View*> Timeline::playedViews() const
{
    return playedViews_;
}

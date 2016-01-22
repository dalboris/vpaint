// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "MainWindow.h"

#include "Scene.h"
#include "View3D.h"
#include "View.h"
#include "MultiView.h"
#include "Timeline.h"
#include "SaveAndLoad.h"
#include "Global.h"
#include "DevSettings.h"
#include "ObjectPropertiesWidget.h"
#include "AnimatedCycleWidget.h"
#include "Background/BackgroundWidget.h"
#include "EditCanvasSizeDialog.h"
#include "ExportPngDialog.h"
#include "AboutDialog.h"
#include "SelectionInfoWidget.h"
#include "XmlStreamWriter.h"
#include "XmlStreamReader.h"
#include "VectorAnimationComplex/VAC.h"
#include "VectorAnimationComplex/InbetweenFace.h"

#include <QCoreApplication>
#include <QApplication>
#include <QtDebug>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QScrollArea>
#include <QDockWidget>
#include <QSettings>
#include <QStandardPaths>
#include <QProgressDialog>
#include <QDesktopServices>
#include <QShortcut>


/*********************************************************************
 *                             Constructor
 */


MainWindow::MainWindow() :
    companyName_("VPaint"),
    appName_("VPaint"),
    appVersion_Major_(1),
    appVersion_Minor_(5),
    appVersion_Stage_("RTM"), // If shipped, must be "RTM"
    appVersion_SubStage_(0),

    scene_(0),
    multiView_(0),

    aboutDialog_(0),

    gettingStarted_(0),
    userManual_(0),

    undoStack_(),
    undoIndex_(-1),

    fileHeader_("---------- Vec File ----------"),
    saveFilename_(),
    autosaveFilename_("0.vec"),
    autosaveTimer_(),
    autosaveIndex_(0),
    autosaveOn_(true),
    autosaveDir_(),

    clipboard_(0),

    view3D_(0),
    timeline_(0),
    selectionInfo_(0),
    exportPngDialog_(0),
    editCanvasSizeDialog_(0),
    exportingPng_(false)
{
    // Global object
    Global::initialize(this);

    // Preferences
    global()->readSettings();
    new DevSettings();

    // Scene
    scene_ = new Scene();

    // Timeline (must exist before multiview is created, so that newly created views can register to timeline)
    timeline_ = new Timeline(scene_, this);
    connect(timeline_, SIGNAL(timeChanged()),
            this, SLOT(updatePicking())); // maybe should avoid that when playing the animation
    connect(timeline_, SIGNAL(timeChanged()),
            this, SLOT(update())); // should be call in same order
    connect(scene(), SIGNAL(changed()),
            timeline_, SLOT(update()));
    connect(scene(),SIGNAL(selectionChanged()),timeline_,SLOT(update()));

    // 2D Views
    multiView_ = new MultiView(scene_, this);
    connect(multiView_, SIGNAL(allViewsNeedToUpdate()), timeline_,SLOT(update()));
    connect(multiView_, SIGNAL(allViewsNeedToUpdate()), this, SLOT(update()));
    connect(multiView_, SIGNAL(allViewsNeedToUpdatePicking()), this, SLOT(updatePicking()));
    setCentralWidget(multiView_); // views are drawn
    connect(multiView_, SIGNAL(activeViewChanged()), this, SLOT(updateViewMenu()));
    connect(multiView_, SIGNAL(activeViewChanged()), timeline_, SLOT(update()));

    connect(multiView_, SIGNAL(settingsChanged()), this, SLOT(updateViewMenu()));

    // 3D View
    view3D_ = new View3D(scene_, 0);
    view3D_->setParent(this, Qt::Window);
    //view3D_->show();
    connect(view3D_, SIGNAL(allViewsNeedToUpdate()), this, SLOT(update()));
    connect(view3D_, SIGNAL(allViewsNeedToUpdatePicking()), this, SLOT(updatePicking()));
    connect(multiView_, SIGNAL(activeViewChanged()), view3D_, SLOT(update()));
    connect(multiView_, SIGNAL(cameraChanged()), view3D_, SLOT(update()));

    // Selection Info
    selectionInfo_ = new SelectionInfoWidget(0);
    connect(scene(),SIGNAL(selectionChanged()),selectionInfo_,SLOT(updateInfo()));
    //selectionInfo_->show();

    // redraw when the scene changes
    connect(scene_, SIGNAL(needUpdatePicking()),
            this, SLOT(updatePicking()));
    connect(scene_, SIGNAL(changed()),
          this, SLOT(update()));

    // redraw when the settings change
    connect(DevSettings::instance(), SIGNAL(changed()),
          this, SLOT(updatePicking())); // hopefully this doesn't occur very often
    connect(DevSettings::instance(), SIGNAL(changed()),
            this, SLOT(update()));

    // initializations
    createActions();
    createDocks();
    createStatusBar();
    createToolbars();
    createMenus();

    // handle undo/redo
    connect(scene_, SIGNAL(checkpoint()),
          this, SLOT(addToUndoStack()));
    addToUndoStack();

    //setFocusPolicy(Qt::ClickFocus);

    // Application name
    QCoreApplication::setApplicationName(appName_);
    setWindowFilePath(tr("New Document"));
    QGuiApplication::setApplicationDisplayName(appName_);

    // Application icon
    QGuiApplication::setWindowIcon(QIcon(":/images/icon-256.png"));

    // Help
    gettingStarted_ = new QTextBrowser(this);
    gettingStarted_->setWindowFlags(Qt::Window);
    //gettingStarted_->setSource(QUrl("help/getting-started.htm"));
    gettingStarted_->setSearchPaths(QStringList("help/"));
    gettingStarted_->setMinimumSize(800,500);

    userManual_ = new QTextBrowser(this);
    userManual_->setWindowFlags(Qt::Window);
    //userManual_->setSource(QUrl("help/user-manual.htm"));
    userManual_->setSearchPaths(QStringList("help/"));
    userManual_->setMinimumSize(800,500);

    // Remove context menu on rightclick
    setContextMenuPolicy(Qt::NoContextMenu);

    // Autosave
    autosaveBegin();
}

void MainWindow::updateObjectProperties()
{
    inspector->setObjects(scene()->getVAC_()->selectedCells());
}

View * MainWindow::activeView() const
{
    return multiView_->activeView();
}

View * MainWindow::hoveredView() const
{
    return multiView_->hoveredView();
}

Timeline * MainWindow::timeline() const
{
    return timeline_;
}

bool MainWindow::isShowCanvasChecked() const
{
    return actionShowCanvas->isChecked();

}
void MainWindow::autosave()
{
    doSave(autosaveDir_.absoluteFilePath(autosaveFilename_));
}

void MainWindow::autosaveBegin()
{
    bool success = true;

    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QDir().mkpath(dataPath);
    QDir dataDir(dataPath);
    if(!dataDir.exists())
        success = false;

    if(success)
    {
        if(!dataDir.exists("autosave"))
        {
            dataDir.mkdir("autosave");
        }
        dataDir.cd("autosave");
        autosaveDir_ = dataDir;
        if(!autosaveDir_.exists())
        {
            success = false;
        }
        else
        {
            QStringList nameFilters;
            nameFilters << "*.vec";
            autosaveDir_.setNameFilters(nameFilters);
            QFileInfoList fileInfoList = autosaveDir_.entryInfoList(QDir::Files, QDir::Name);
            if(fileInfoList.isEmpty())
            {
                autosaveIndex_ = 0;
            }
            else
            {
                QString filename = fileInfoList.last().fileName();
                QStringList splitted = filename.split('.');
                if(splitted.size() < 2)
                {
                    qDebug() << "Warning: autosaved file matching *.vec has been found, but failed to be split into %1.vec";
                    autosaveIndex_ = 0;
                }
                else
                {
                    int lastIndex = splitted.first().toInt();
                    autosaveIndex_ = lastIndex + 1;
                }
            }
            autosaveFilename_.setNum(autosaveIndex_);
            autosaveFilename_ += QString(".vec");
            while(autosaveDir_.exists(autosaveFilename_))
            {
                autosaveIndex_++;
                autosaveFilename_.setNum(autosaveIndex_);
                autosaveFilename_ += QString(".vec");
            }
        }
    }

    if(success)
    {
        autosaveOn_ = true;
        autosaveTimer_.setInterval(60000); // every minute
        connect(&autosaveTimer_,SIGNAL(timeout()), this, SLOT(autosave()));
        autosaveTimer_.start();
    }
    else
    {
        autosaveOn_ = false;
    }

    /*
    QSettings settings(companyName_, appName_);
    bool hasCrashed = settings.value("has-crashed",false).toBool();
    if(hasCrashed)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      "Crash",
                                      "Oops... it seems that VPaint has crashed during its previous execution :-(\n "
                                      "We are terribly sorry and will do our best to fix the issue in future releases.\n\n"
                                      "Fortunately, VPaint has saved your work before crashing. Do you want to open it?\n\n"
                                      "Note: this message also appears if another instance of VPaint is running. In this case, press \"No\".",
                                        QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            doOpen(autosaveFilename_);
        }
    }
    settings.setValue("has-crashed",true);
    autosaveTimer_.setInterval(1000);
    connect(&autosaveTimer_,SIGNAL(timeout()), this, SLOT(autosave()));
    autosaveTimer_.start();
    */

}

void MainWindow::autosaveEnd()
{
    if(autosaveOn_)
    {
        autosaveDir_.remove(autosaveFilename_);
    }
}

MainWindow::~MainWindow()
{
    typedef QPair<QDir,Scene*> UndoItem;
    foreach(UndoItem p, undoStack_)
        delete p.second;
    delete scene_;

    autosaveEnd();
}

Scene * MainWindow::scene() const
{
    return scene_;
}


void MainWindow::addToUndoStack()
{
    undoIndex_++;
    for(int j=undoStack_.size()-1; j>=undoIndex_; j--)
    {
        delete undoStack_[j].second;
        undoStack_.removeLast();
    }
    undoStack_ << qMakePair(global()->documentDir(), new Scene());
    undoStack_[undoIndex_].second->copyFrom(scene_);
}

void MainWindow::goToUndoIndex_(int undoIndex)
{
    // Set new undo index
    undoIndex_ = undoIndex;

    // Remap relative paths in history
    if (undoStack_[undoIndex].first != global()->documentDir())
    {
        undoStack_[undoIndex].second->relativeRemap(
            undoStack_[undoIndex_].first,
            global()->documentDir());
        undoStack_[undoIndex].first = global()->documentDir();
    }

    // Set scene data from undo history
    scene_->copyFrom(undoStack_[undoIndex].second);
}

void MainWindow::undo()
{
    if(undoIndex_>0)
    {
        goToUndoIndex_(undoIndex_ - 1);
    }
    else
    {
        statusBar()->showMessage(tr("Nothing to undo"));
    }
}

void MainWindow::redo()
{
    if(undoIndex_<undoStack_.size()-1)
    {
        goToUndoIndex_(undoIndex_ + 1);
    }
    else
    {
        statusBar()->showMessage(tr("Nothing to redo"));
    }
}

void MainWindow::cut()
{
    scene_->cut(clipboard_);
}

void MainWindow::copy()
{
    scene_->copy(clipboard_);
}

void MainWindow::paste()
{
    scene_->paste(clipboard_);
}

void MainWindow::motionPaste()
{
    scene_->motionPaste(clipboard_);
}

void MainWindow::editAnimatedCycle(VectorAnimationComplex::InbetweenFace * inbetweenFace, int indexCycle)
{
    // Make this animated cycle the one edited in the editor
    animatedCycleEditor->setAnimatedCycle(inbetweenFace,indexCycle);

    // Show editor
    if(!dockAnimatedCycleEditor->isVisible())
    {
        //if(dockAnimatedCycleEditor->isFloating)
        //addDockWidget(Qt::RightDockWidgetArea, dockAnimatedCycleEditor);
        dockAnimatedCycleEditor->show();
    }
}

void MainWindow::createInbetweenFace()
{
    // Create inbetween face with one (invalid for now) animated cycle
    InbetweenFace * inbetweenFace = scene_->createInbetweenFace();
    inbetweenFace->addAnimatedCycle();

    // Set as edited cycle
    editAnimatedCycle(inbetweenFace,0);
}


void MainWindow::displayModeChanged()
{
    updatePicking();
    update();
}

void MainWindow::setDisplayModeNormal()
{
    multiView_->setDisplayMode(ViewSettings::ILLUSTRATION);
}

void MainWindow::setDisplayModeNormalOutline()
{
    multiView_->setDisplayMode(ViewSettings::ILLUSTRATION_OUTLINE);
}

void MainWindow::setDisplayModeOutline()
{
    multiView_->setDisplayMode(ViewSettings::OUTLINE);
}

void MainWindow::setOnionSkinningEnabled(bool enabled)
{
    multiView_->setOnionSkinningEnabled(enabled);
}



void MainWindow::toggleShowCanvas(bool)
{
    update();
}

bool MainWindow::isEditCanvasSizeVisible() const
{
    bool res = false;

    if(editCanvasSizeDialog_)
        res = res || editCanvasSizeDialog_->isVisible();

    if(exportPngDialog_)
        res = res || exportPngDialog_->isVisible();

    if(exportingPng_)
        res = true;

    return res;
}

void MainWindow::editCanvasSize()
{
    if(isEditCanvasSizeVisible())
        return;

    if(!editCanvasSizeDialog_)
    {
        editCanvasSizeDialog_ = new EditCanvasSizeDialog(scene());
        editCanvasSizeDialog_->setParent(this, Qt::Dialog);
        editCanvasSizeDialog_->setModal(false);
    }

    if(!actionShowCanvas->isChecked())
        actionShowCanvas->setChecked(true);

    editCanvasSizeDialog_->show();
}

/*********************************************************************
 *                       Overloaded event methods
 */


void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // Early catch of overloaded standard shortcut to prevent the "ambiguous shortcut" popup to be shown

    // ignore the event
    event->ignore();
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    // ignore the event
    event->ignore();
}

void MainWindow::update()
{
    multiView_->update();
    if(view3D_ && view3D_->isVisible())
    {
        view3D_->update();
    }
}

void MainWindow::updatePicking()
{
    multiView_->updatePicking();
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    qDebug() << "event filter";

    if(event->type() == QEvent::Shortcut)
    {
        qDebug() << "Shortcut event";
    }
    return QMainWindow::eventFilter(object, event);
}



/*********************************************************************
 *                     Save / Load / Close
 */


void MainWindow::closeEvent(QCloseEvent *event)
{
      if (close())
      {
        global()->writeSettings();
        event->accept();
        selectionInfo_->close();
      }
      else
      {
          event->ignore();
      }
}

bool MainWindow::close()
{
    // TODO: ask are you sure? for unsaved document
    return true;
}

bool MainWindow::newDocument()
{
    // First, close the current document
    if(!close())
        return false;

    // If success, proceed
    setSaveFilename_(QString());
    Scene * newScene = new Scene();
    scene_->copyFrom(newScene);
    addToUndoStack();
    delete newScene;
    setWindowFilePath(tr("New Document"));
    return true;
}

bool MainWindow::open()
{
    //First, close the current document
    if(!close())
    {
        return false;
    }

    // Browse for a file to open
    QString filename = QFileDialog::getOpenFileName(
        this, tr("Open"), QStandardPaths::writableLocation(QStandardPaths::PicturesLocation), tr("Vec files (*.vec)"));

    // Set save filename.
    //
    // This must be done *before* calling doOpen() because doOpen() will cause
    // the scene to change which will cause a redraw, which requires the save
    // filename to be set to resolve relative file paths
    QString oldFilename = saveFilename_;
    setSaveFilename_(filename);

    // Try to open file
    bool success = doOpen(filename);

    // Set a few things depending on success
    if(success)
    {
        setSaveFilename_(filename);
        setWindowFilePath(filename);
        return true;
    }
    else
    {
        setSaveFilename_(oldFilename);
        return false;
    }
}

bool MainWindow::save()
{
    if(saveFilename_.isEmpty())
    {
        return saveAs();
    }
    else
    {
        bool success = doSave(saveFilename_);

        if(success)
        {
            statusBar()->showMessage(tr("File %1 successfully saved.").arg(saveFilename_));
            setWindowFilePath(saveFilename_);
            return true;
        }
        else
        {
            QMessageBox::warning(this, tr("Error"), tr("File %1 not saved: couldn't write file").arg(saveFilename_));
            return false;
        }
    }
}

bool MainWindow::saveAs()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Save As"), QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));

    if (filename.isEmpty())
        return false;

    if(!filename.endsWith(".vec"))
        filename.append(".vec");

    bool relativeRemap = true;
    bool success = doSave(filename, relativeRemap);

    if(success)
    {
        statusBar()->showMessage(tr("File %1 successfully saved.").arg(filename));
        setSaveFilename_(filename);
        setWindowFilePath(filename);
        return true;
    }
    else
    {
        QMessageBox::warning(this, tr("Error"), tr("File %1 not saved: couldn't write file").arg(filename));
        return false;
    }
}

bool MainWindow::exportSVG()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Export as SVG"), QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    if (filename.isEmpty())
        return false;

    if(!filename.endsWith(".svg"))
        filename.append(".svg");

    bool success = doExportSVG(filename);

    if(success)
    {
        return true;
    }
    else
    {
        QMessageBox::warning(this, tr("Error"), tr("File %1 not saved: couldn't write file").arg(filename));
        return false;
    }
}

bool MainWindow::exportPNG()
{
    exportPngFilename_ = QFileDialog::getSaveFileName(this, tr("Export as PNG"), QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    if (exportPngFilename_.isEmpty())
        return false;

    if(!exportPngFilename_.endsWith(".png"))
        exportPngFilename_.append(".png");

    if(!exportPngDialog_)
    {
        exportPngDialog_ = new ExportPngDialog(scene());
        exportPngDialog_->setParent(this, Qt::Dialog);
        exportPngDialog_->setModal(false);
        connect(exportPngDialog_, SIGNAL(accepted()), this, SLOT(acceptExportPNG()));
        connect(exportPngDialog_, SIGNAL(rejected()), this, SLOT(rejectExportPNG()));
    }

    exportPngCanvasWasVisible_ = actionShowCanvas->isChecked();
    if(!exportPngCanvasWasVisible_)
        actionShowCanvas->setChecked(true);

    exportPngDialog_->show();

    // Note: the dialog is modeless to allow user to pan/zoom the image while changing
    //       canvas size and resolution.
    //       But this mean that we can't return here whether or not the export was done

    // The return value doesn't actually make sense here. Maybe this function
    // shouldn't return anything instead.
    return true;
}

bool MainWindow::acceptExportPNG()
{
    exportingPng_ = true; // This is necessary so that isEditCanvasSizeVisible() returns true
                          // so that global()->toolMode() returns EDIT_CANVAS_SIZE so that
                          // selection is not rendered as selected
    bool success = doExportPNG(exportPngFilename_);
    exportingPng_ = false;


    if(!success)
    {
        QMessageBox::warning(this, tr("Error"), tr("File %1 not saved: couldn't write file").arg(exportPngFilename_));
    }

    if(!exportPngCanvasWasVisible_)
        actionShowCanvas->setChecked(false);

    updatePicking();
    update();

    return success;
}

bool MainWindow::rejectExportPNG()
{
    if(!exportPngCanvasWasVisible_)
        actionShowCanvas->setChecked(false);

    updatePicking();
    update();

    return false;
}

void MainWindow::setSaveFilename_(const QString & filename)
{
    saveFilename_ = filename;

    QFileInfo fileInfo(filename);
    if (fileInfo.exists() && fileInfo.isFile())
    {
        global()->setDocumentDir(fileInfo.dir());
    }
    else
    {
        global()->setDocumentDir(QDir::home());
    }
}

bool MainWindow::doOpen(const QString & filename)
{
    // Open file
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug() << "Error: cannot open file";
        QMessageBox::warning(this, tr("Error"), tr("Error: couldn't open file %1").arg(filename));
        return false;
    }

    // Read first char to determine in which format the file is written
    QTextStream in(&file);
    QChar firstChar;
    in >> firstChar;

    // Re-open file to read from scratch
    file.close();
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug() << "Error: cannot open file";
        QMessageBox::warning(this, tr("Error"), tr("Error: couldn't open file %1").arg(filename));
        return false;
    }

    // Determine file format and read accordingly
    if (firstChar == '-') // Pre-2015 YAML-like deprecated format
    {
        QTextStream in(&file);
        read_DEPRECATED(in);
    }
    else // 2015+ XML format
    {
        XmlStreamReader xml(&file);
        read(xml);
    }

    // Close file
    file.close();

    // Add to undo stack
    addToUndoStack();

    // Success
    return true;
}

bool MainWindow::doSave(const QString & filename, bool relativeRemap)
{
    // Open file to save to
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QFile::Truncate | QFile::Text))
    {
        qWarning("Couldn't write file.");
        return false;
    }

    // Remap relative paths if need be
    if (relativeRemap)
    {
        QFileInfo fileInfo(file);
        QDir oldDocumentDir = global()->documentDir();
        QDir newDocumentDir = fileInfo.dir();
        if (oldDocumentDir != newDocumentDir)
        {
            global()->setDocumentDir(newDocumentDir);
            scene()->relativeRemap(oldDocumentDir, newDocumentDir);
        }
    }

    // Write to file
    XmlStreamWriter xmlStream(&file);
    write(xmlStream);

    // Close file
    file.close();

    // Success
    return true;
}

void MainWindow::read_DEPRECATED(QTextStream & in)
{
    // Buffer variables
    QChar cskip;
    QString field;

    // Header
    QString header = in.readLine();
    if(header != fileHeader_)
        QMessageBox::warning(this, tr("Warning"), tr("Incorrect file header. I'm still trying to open the file but it might be corrupted."));

    // Version
    int minor, major;
    field = Read::field(in);
    in >> major >> cskip >> minor;
    if(major!=1 || minor!=0)
        QMessageBox::warning(this, tr("Warning"), tr("Incorrect file version. I'm still trying to open the file but it might be corrupted."));

    // Scene
    field = Read::field(in);
    Read::skipBracket(in);
    scene_->read(in);
    Read::skipBracket(in);
}

void MainWindow::write_DEPRECATED(QTextStream & out)
{
    Save::resetIndent();

    // Header
    out << fileHeader_;

    // Version
    out << Save::newField("Version")
        << 1 << "." << 0;

    // Scene
    out << Save::newField("Scene");
    out << Save::openCurlyBrackets();
    scene_->save(out);
    out << Save::closeCurlyBrackets();
}

void MainWindow::write(XmlStreamWriter &xml)
{
    // Start XML Document
    xml.writeStartDocument();

    // Header
    xml.writeComment(" Created with VPaint (http://www.vpaint.org) ");
    xml.writeCharacters("\n\n");

    // Document
    xml.writeStartElement("vec");
    {
        xml.writeAttribute("version", "1.0");

        // Metadata such as author and license? Different options:
        //   1) as comments in header (issue: not part of document or XML spec, cross-editor compatibility issues)
        //   2) as attributes of vec
        //   3) as its own XML element
        // "metadata" or "properties"? Probably metadata. even in PDF when this info is often accessed
        // in File > Properties, it is still sotred as "metadata"
        // Resources:
        //   https://helpx.adobe.com/acrobat/using/pdf-properties-metadata.html)
        //   http://www.w3.org/TR/SVG/metadata.html
        //xmlStream.writeStartElement("metadata");
        //xmlStream.writeAttribute("author", "Boris Dalstein");
        //xmlStream.writeAttribute("license", "CC BY-SA");
        //xmlStream.writeEndElement();

        // Playback
        xml.writeStartElement("playback");
        timeline()->write(xml);
        xml.writeEndElement();

        // Canvas
        xml.writeStartElement("canvas");
        scene()->writeCanvas(xml);
        xml.writeEndElement();

        // Layer
        xml.writeStartElement("layer");
        scene()->write(xml);
        xml.writeEndElement();
    }
    xml.writeEndElement();

    // End XML Document
    xml.writeEndDocument();
}

void MainWindow::read(XmlStreamReader & xml)
{
    if (xml.readNextStartElement())
    {
        if (xml.name() != "vec")
        {
            QMessageBox::warning(this,
                "Cannot open file",
                "Sorry, the file you are trying to open is an invalid VEC file.");
            return;
        }

        if(xml.attributes().value("version") != "1.0")
        {
            QMessageBox::warning(this,
                "File version more recent than VPaint",
                "The file you are trying to open has been created with a "
                "version of VPaint more recent than the one you are using. "
                "We will still try to open it, but errors may occur, or it "
                "may not be displayed at intended. We recommend to download "
                "the latest version of VPaint at www.vpaint.org");
        }

        int numLayer = 0;
        while (xml.readNextStartElement())
        {
            // Playback
            if (xml.name() == "playback")
            {
                timeline_->read(xml);
            }

            // Canvas
            else if (xml.name() == "canvas")
            {
                scene_->readCanvas(xml);
            }

            // Layer
            else if (xml.name() == "layer")
            {
                // For now, only supports one layer, i.e., it reads the first one and
                // ignore all the others
                ++numLayer;
                if(numLayer == 1)
                {
                    scene_->read(xml);
                }
                else
                {
                    xml.skipCurrentElement();
                }
            }

            // Unknown
            else
            {
                xml.skipCurrentElement();
            }
        }
    }
}

bool MainWindow::doExportSVG(const QString & filename)
{
    QFile data(filename);
    if (data.open(QFile::WriteOnly | QFile::Truncate | QFile::Text)) {

        QTextStream out(&data);

        QString header = QString(
                "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
                "<!-- Created with VPaint (http://www.vpaint.org/) -->\n\n"

                "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n"
                "  \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n"
                "<svg \n"
                "  viewBox=\"%1 %2 %3 %4\"\n"
                "  xmlns=\"http://www.w3.org/2000/svg\"\n"
                "  xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n")

                .arg(scene_->left())
                .arg(scene_->top())
                .arg(scene_->width())
                .arg(scene_->height());

        QString footer = "</svg>";

        out << header;
        scene_->exportSVG(multiView_->activeView()->activeTime(), out);
        out << footer;

        statusBar()->showMessage(tr("File %1 successfully saved.").arg(filename));
        return true;
    }
    else
    {
        qDebug() << "Error: cannot open file";
        return false;
    }
}

bool MainWindow::doExportPNG(const QString & filename)
{
    if(!exportPngDialog_->exportSequence())
    {
        // Export single frame

        QImage img = multiView_->activeView()->drawToImage(
                    scene()->left(), scene()->top(), scene()->width(), scene()->height(),
                    exportPngDialog_->pngWidth(), exportPngDialog_->pngHeight());

        img.save(filename);
    }
    else
    {
        // Export sequence of frames

        // Decompose filename into basename + suffix. Example:
        //     abc_1234_5678.de.png  ->   abc_1234  +  de.png
        QFileInfo info(filename);
        QString baseName = info.baseName();
        QString suffix = info.suffix();
        // Decompose basename into cleanedbasename + numbering. Examples:
        //     abc_1234_5678  ->     abc_1234 + 5678
        int iNumbering = baseName.indexOf(QRegExp("_[0-9]*$"));
        if(iNumbering != -1)
        {
            baseName.chop(baseName.length() - iNumbering);
        }

        // Get dir
        QDir dir = info.absoluteDir();

        // Get and delete files from previous export
        QString nameFilter = baseName + QString("_*.") +  suffix;
        QStringList nameFilters = QStringList() << nameFilter;
        QStringList prevFiles = dir.entryList(nameFilters, QDir::Files);
        foreach(QString prevFile, prevFiles)
            dir.remove(prevFile);

        // Get frame numbers to export
        int firstFrame = timeline()->firstFrame();
        int lastFrame = timeline()->lastFrame();

        // Create Progress dialog for feedback
        QProgressDialog progress("Export sequence as PNGs...", "Abort", 0, lastFrame-firstFrame+1, this);
        progress.setWindowModality(Qt::WindowModal);

        // Export all frames in the sequence
        for(int i=firstFrame; i<=lastFrame; ++i)
        {
            progress.setValue(i-firstFrame);

            if (progress.wasCanceled())
                break;

            QString number = QString("%1").arg(i, 4, 10, QChar('0'));
            QString filePath = dir.absoluteFilePath(
                        baseName + QString("_") + number + QString(".") + suffix);

            QImage img = multiView_->activeView()->drawToImage(
                        Time(i),
                        scene()->left(), scene()->top(), scene()->width(), scene()->height(),
                        exportPngDialog_->pngWidth(), exportPngDialog_->pngHeight());

            img.save(filePath);
        }
        progress.setValue(lastFrame-firstFrame+1);

    }

    return true;
}

void MainWindow::onlineDocumentation()
{
    QDesktopServices::openUrl(QUrl("http://www.vpaint.org/doc"));
}

void MainWindow::gettingStarted()
{
    gettingStarted_->setSource(QUrl("help/getting-started.htm"));
    gettingStarted_->show();
}

void MainWindow::manual()
{
    gettingStarted_->setSource(QUrl("help/user-manual.htm"));
    userManual_->show();
}

void MainWindow::about()
{
    if(!aboutDialog_)
    {
        aboutDialog_ = new AboutDialog(global()->settings().showAboutDialogAtStartup());
        aboutDialog_->setParent(this, Qt::Dialog);
    }

    aboutDialog_->exec();

    if(aboutDialog_)
    {
        global()->settings().setShowAboutDialogAtStartup(aboutDialog_->showAtStartup());
    }


    /*
    QString copyrightLine = "Copyright (C) 2015 Boris Dalstein";

    QString licenseLine =
            "Permission is hereby granted, free of charge, to any person obtaining a copy"
            " of this software and associated documentation files (the \"Software\"), to deal"
            " in the Software without restriction, including without limitation the rights"
            " to use, copy, modify, merge, publish, distribute, sublicense, and/or sell"
            " copies of the Software, and to permit persons to whom the Software is"
            " furnished to do so, subject to the following conditions:\n\n"
            " "
            " The above copyright notice and this permission notice shall be included in"
            " all copies or substantial portions of the Software.\n\n"
            " "
            " THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR"
            " IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,"
            " FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE"
            " AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER"
            " LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,"
            " OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN"
            " THE SOFTWARE.";


    QString versionLine = QString("VPaint 1.5");//.arg(appVersion_Major_).arg(appVersion_Minor_);
    //if(appVersion_Stage_ != "RTM")
    //    versionLine +=
    //            QString(" (") + appVersion_Stage_ +
    //            QString(" ") + QString::number(appVersion_SubStage_) +
    //            QString(")");

    QString aboutLine =
            QString("VPaint is an innovative yet simple vector-based painting program. "
                    "For more information, visit:\n\n http://www.vpaint.org");

    QMessageBox::about(
                this, tr("About VPaint"),
                copyrightLine + "\n\n" +
                versionLine + "\n\n" +
                aboutLine);
                */
}

void MainWindow::openClose3D()
{
    if(view3D_)
    {
        if(view3D_->isVisible())
        {
            view3D_->hide();
        }
        else
        {
            view3D_->show();
        }
    }

    updateView3DActionCheckState();
}

void MainWindow::updateView3DActionCheckState()
{
    if(view3D_)
    {
        if(view3D_->isVisible())
        {
            view3DActionSetChecked();
        }
        else
        {
            view3DActionSetUnchecked();
        }
    }
}

void MainWindow::view3DActionSetUnchecked()
{
    actionOpenClose3D->setChecked(false);
}

void MainWindow::view3DActionSetChecked()
{
    actionOpenClose3D->setChecked(true);
}

void MainWindow::openClose3DSettings()
{
    if(view3D_)
    {
        if(view3D_->view3DSettingsWidget()->isVisible())
        {
            view3D_->hide();
        }
        else
        {
            view3D_->openViewSettings();
        }
    }

    updateView3DActionCheckState();

}

void MainWindow::updateView3DSettingsActionCheckState()
{
    if(view3D_)
    {
        if(view3D_->view3DSettingsWidget()->isVisible())
        {
            view3DSettingsActionSetChecked();
        }
        else
        {
            view3DSettingsActionSetUnchecked();
        }
    }
}

void MainWindow::view3DSettingsActionSetUnchecked()
{
    actionOpenView3DSettings->setChecked(false);
}

void MainWindow::view3DSettingsActionSetChecked()
{
    actionOpenView3DSettings->setChecked(true);
}

void MainWindow::updateViewMenu()
{
    // Display mode
    ViewSettings::DisplayMode mode = multiView_->activeView()->viewSettings().displayMode();
    switch(mode)
    {
    case ViewSettings::ILLUSTRATION:
        actionDisplayModeNormal->setChecked(true);
        break;
    case ViewSettings::ILLUSTRATION_OUTLINE:
        actionDisplayModeNormalOutline->setChecked(true);
        break;
    case ViewSettings::OUTLINE:
        actionDisplayModeOutline->setChecked(true);
        break;
    }

    // Onion skinning
    actionOnionSkinning->setChecked(multiView_->activeView()->viewSettings().onionSkinningIsEnabled());
}

/*********************************************************************
 *                             Actions
 */


void MainWindow::createActions()
{
    ///////////////        FILE        ///////////////

    // New
    actionNew = new QAction(/*QIcon(":/iconLoad"),*/ tr("&New"), this);
    actionNew->setStatusTip(tr("Create a new file."));
    actionNew->setShortcut(QKeySequence::New);
    connect(actionNew, SIGNAL(triggered()), this, SLOT(newDocument()));

    // Open
    actionOpen = new QAction(/*QIcon(":/iconLoad"),*/ tr("&Open..."), this);
    actionOpen->setStatusTip(tr("Open an existing file."));
    actionOpen->setShortcut(QKeySequence::Open);
    connect(actionOpen, SIGNAL(triggered()), this, SLOT(open()));

    // Save
    actionSave = new QAction(/*QIcon(":/iconSave"),*/ tr("&Save"), this);
    actionSave->setStatusTip(tr("Save current illustration."));
    actionSave->setShortcut(QKeySequence::Save);
    connect(actionSave, SIGNAL(triggered()), this, SLOT(save()));

    // Save As
    actionSaveAs = new QAction(/*QIcon(":/iconSave"),*/ tr("Save &As..."), this);
    actionSaveAs->setStatusTip(tr("Save current illustration with a new name."));
    actionSaveAs->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S));
    connect(actionSaveAs, SIGNAL(triggered()), this, SLOT(saveAs()));

    // Export SVG
    actionExportSVG = new QAction(/*QIcon(":/iconSave"),*/ tr("SVG (frame) [Beta]"), this);
    actionExportSVG->setStatusTip(tr("Save the current illustration in the SVG file format."));
    //actionExportSVG->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
    connect(actionExportSVG, SIGNAL(triggered()), this, SLOT(exportSVG()));

    // Export PNG
    actionExportPNG = new QAction(/*QIcon(":/iconSave"),*/ tr("PNG (frame or sequence)"), this);
    actionExportPNG->setStatusTip(tr("Save the current illustration in the PNG file format."));
    //actionExportPNG->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
    connect(actionExportPNG, SIGNAL(triggered()), this, SLOT(exportPNG()));

    // Preferences
    /*
    actionPreferences = new QAction(tr("&Preferences..."), this);
    actionPreferences->setStatusTip(tr("Modify the settings of VPaint."));
    actionPreferences->setShortcut(QKeySequence::Preferences);
    connect(actionPreferences, SIGNAL(triggered()), global(), SLOT(openPreferencesDialog()));
    */

    // Quit
    actionQuit = new QAction(/*QIcon(":/iconQuit"),*/ tr("&Quit"), this);
    actionQuit->setStatusTip(tr("Quit VPaint."));
    actionQuit->setShortcut(QKeySequence::Quit);
    connect(actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));


    ///////////////        EDIT        ///////////////

    // Undo
    actionUndo = new QAction(/*QIcon(":/iconUndo"),*/ tr("&Undo"), this);
    actionUndo->setStatusTip(tr("Undo the last action."));
    actionUndo->setShortcut(QKeySequence::Undo);
    connect(actionUndo, SIGNAL(triggered()), this, SLOT(undo()));

    // Redo
    actionRedo = new QAction(/*QIcon(":/iconRedo"),*/ tr("&Redo"), this);
    actionRedo->setStatusTip(tr("Redo an undone action."));
    actionRedo->setShortcut(QKeySequence::Redo);
    connect(actionRedo, SIGNAL(triggered()), this, SLOT(redo()));

    // Cut
    actionCut = new QAction(tr("Cut"), this);
    actionCut->setStatusTip(tr("Move selected objects to the clipboard."));
    actionCut->setShortcut(QKeySequence::Cut);
    connect(actionCut, SIGNAL(triggered()), this, SLOT(cut()));

    // Copy
    actionCopy = new QAction(tr("Copy"), this);
    actionCopy->setStatusTip(tr("Copy the selected objects to the clipboard."));
    actionCopy->setShortcut(QKeySequence::Copy);
    connect(actionCopy, SIGNAL(triggered()), this, SLOT(copy()));

    // Paste
    actionPaste = new QAction(tr("Paste"), this);
    actionPaste->setStatusTip(tr("Paste the objects from the clipboard."));
    actionPaste->setShortcut(QKeySequence::Paste);
    connect(actionPaste, SIGNAL(triggered()), this, SLOT(paste()));


    // Smart Delete
    actionSmartDelete = new QAction(tr("Delete"), this);
    actionSmartDelete->setStatusTip(tr("Delete the selected objects, merging adjacent objects when possible."));
#ifdef Q_OS_MAC
    actionSmartDelete->setShortcut(QKeySequence(Qt::Key_Delete));
#else
    actionSmartDelete->setShortcut(QKeySequence::Delete);
#endif
    actionSmartDelete->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionSmartDelete, SIGNAL(triggered()), scene_, SLOT(smartDelete()));

    // Hard Delete
    actionHardDelete = new QAction(tr("Hard Delete"), this);
    actionHardDelete->setStatusTip(tr("Delete the selected objects and adjacent objects together."));
    actionHardDelete->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Delete));
    actionHardDelete->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionHardDelete, SIGNAL(triggered()), scene_, SLOT(deleteSelectedCells()));

    // Hard Delete
    actionTest = new QAction(tr("Test"), this);
    actionTest->setStatusTip(tr("For development tests: quick and dirty function."));
    actionTest->setShortcut(QKeySequence(Qt::Key_T));
    actionTest->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionTest, SIGNAL(triggered()), scene_, SLOT(test()));

    ///////////////        VIEW        ///////////////

    // Zoom In
    actionZoomIn = new QAction(tr("Zoom in"), this);
    actionZoomIn->setStatusTip(tr("Makes objects appear bigger."));
    actionZoomIn->setShortcut(QKeySequence::ZoomIn);
    actionZoomIn->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionZoomIn, SIGNAL(triggered()), multiView_, SLOT(zoomIn()));

    // Zoom Out
    actionZoomOut = new QAction(tr("Zoom out"), this);
    actionZoomOut->setStatusTip(tr("Makes objects appear smaller."));
    actionZoomOut->setShortcut(QKeySequence::ZoomOut);
    actionZoomOut->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionZoomOut, SIGNAL(triggered()), multiView_, SLOT(zoomOut()));

    actionShowCanvas = new QAction(tr("Display canvas"), this);
    actionShowCanvas->setStatusTip(tr("Show or hide the canvas borders."));
    actionShowCanvas->setCheckable(true);
    actionShowCanvas->setChecked(true);
    //actionShowCanvas->setShortcut(QKeySequence::ZoomOut);
    //actionShowCanvas->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionShowCanvas, SIGNAL(triggered(bool)), this, SLOT(toggleShowCanvas(bool)));

    actionEditCanvasSize = new QAction(tr("Edit canvas size..."), this);
    actionEditCanvasSize->setStatusTip(tr("Edit the size of the canvas."));
    //actionEditCanvasSize->setShortcut(QKeySequence::ZoomOut);
    //actionEditCanvasSize->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionEditCanvasSize, SIGNAL(triggered()), this, SLOT(editCanvasSize()));

    // Fit Illustration In Window
    actionFitAllInWindow = new QAction(tr("Fit illustration in window"), this);
    actionFitAllInWindow->setStatusTip(tr("Automatically select an appropriate zoom to see the whole illustration."));
    //actionFitAllInWindow->setShortcut(QKeySequence::ZoomOut);
    //actionFitAllInWindow->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionFitAllInWindow, SIGNAL(triggered()), multiView_, SLOT(fitAllInWindow()));

    // Fit Selection In Window
    actionFitSelectionInWindow = new QAction(tr("Fit selection in window"), this);
    actionFitSelectionInWindow->setStatusTip(tr("Automatically select an appropriate zoom to see the selected objects."));
    //actionFitSelectionInWindow->setShortcut(QKeySequence::ZoomOut);
    //actionFitSelectionInWindow->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionFitSelectionInWindow, SIGNAL(triggered()), multiView_, SLOT(fitSelectionInWindow()));

    actionToggleOutline = new QAction(tr("Toggle outline"), this);
    actionToggleOutline->setStatusTip(tr("Toggle the outline of the illustration"));
    actionToggleOutline->setShortcut(QKeySequence(Qt::Key_Space));
    actionToggleOutline->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionToggleOutline, SIGNAL(triggered()), multiView_, SLOT(toggleOutline()));

    actionToggleOutlineOnly = new QAction(tr("Toggle only outline"), this);
    actionToggleOutlineOnly->setStatusTip(tr("Toggle only the outline of the illustration"));
    actionToggleOutlineOnly->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Space));
    actionToggleOutlineOnly->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionToggleOutlineOnly, SIGNAL(triggered()), multiView_, SLOT(toggleOutlineOnly()));

    actionDisplayModeNormal = new QAction(tr("Normal"), this);
    actionDisplayModeNormal->setCheckable(true);
    actionDisplayModeNormal->setStatusTip(tr("Switch to normal display mode for the active view"));
    actionDisplayModeNormal->setShortcut(QKeySequence(Qt::Key_1));
    actionDisplayModeNormal->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionDisplayModeNormal, SIGNAL(triggered()), this, SLOT(setDisplayModeNormal()));

    actionDisplayModeNormalOutline = new QAction(tr("Normal+Outline"), this);
    actionDisplayModeNormalOutline->setCheckable(true);
    actionDisplayModeNormalOutline->setStatusTip(tr("Switch to normal+outline display mode for the active view"));
    actionDisplayModeNormalOutline->setShortcut(QKeySequence(Qt::Key_2));
    actionDisplayModeNormalOutline->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionDisplayModeNormalOutline, SIGNAL(triggered()), this, SLOT(setDisplayModeNormalOutline()));

    actionDisplayModeOutline = new QAction(tr("Outline"), this);
    actionDisplayModeOutline->setCheckable(true);
    actionDisplayModeOutline->setStatusTip(tr("Switch to outline display mode for the active view"));
    actionDisplayModeOutline->setShortcut(QKeySequence(Qt::Key_3));
    actionDisplayModeOutline->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionDisplayModeOutline, SIGNAL(triggered()), this, SLOT(setDisplayModeOutline()));

    QActionGroup * displayModeGroup = new QActionGroup(this);
    displayModeGroup->addAction(actionDisplayModeNormal);
    displayModeGroup->addAction(actionDisplayModeNormalOutline);
    displayModeGroup->addAction(actionDisplayModeOutline);
    actionDisplayModeNormal->setChecked(true);

    actionOnionSkinning = new QAction(tr("Onion skinning"), this);
    actionOnionSkinning->setCheckable(true);
    actionOnionSkinning->setChecked(false);
    actionOnionSkinning->setStatusTip(tr("Toggle the display of onion skins"));
    actionOnionSkinning->setShortcut(QKeySequence(Qt::Key_O));
    actionOnionSkinning->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionOnionSkinning, SIGNAL(triggered(bool)), this, SLOT(setOnionSkinningEnabled(bool)));

    actionOpenView3DSettings = new QAction(tr("3D View Settings [Beta]"), this);
    actionOpenView3DSettings->setCheckable(true);
    actionOpenView3DSettings->setStatusTip(tr("Open the settings dialog for the 3D view"));
    //actionOpenView3DSettings->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_5));
    actionOpenView3DSettings->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionOpenView3DSettings, SIGNAL(triggered()), view3D_, SLOT(openViewSettings()));
    connect(view3D_->view3DSettingsWidget(), SIGNAL(closed()), this, SLOT(view3DSettingsActionSetUnchecked()));

    actionOpenClose3D = new QAction(tr("3D View [Beta]"), this);
    actionOpenClose3D->setCheckable(true);
    actionOpenClose3D->setStatusTip(tr("Open or Close the 3D inbetween View"));
    connect(actionOpenClose3D, SIGNAL(triggered()), this, SLOT(openClose3D()));
    connect(view3D_, SIGNAL(closed()), this, SLOT(view3DActionSetUnchecked()));


    // Splitting
    actionSplitClose = new QAction(tr("Close active view"), this);
    actionSplitClose->setStatusTip(tr("Close the active view"));
    actionSplitClose->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_0));
    actionSplitClose->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionSplitClose, SIGNAL(triggered()), multiView_, SLOT(splitClose()));

    actionSplitOne = new QAction(tr("Close all but active view"), this);
    actionSplitOne->setStatusTip(tr("Close all views except the active view"));
    actionSplitOne->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_1));
    actionSplitOne->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionSplitOne, SIGNAL(triggered()), multiView_, SLOT(splitOne()));

    actionSplitVertical = new QAction(tr("Split view vertically"), this);
    actionSplitVertical->setStatusTip(tr("Split the active view vertically"));
    actionSplitVertical->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_2));
    actionSplitVertical->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionSplitVertical, SIGNAL(triggered()), multiView_, SLOT(splitVertical()));

    actionSplitHorizontal = new QAction(tr("Split view horizontally"), this);
    actionSplitHorizontal->setStatusTip(tr("Split the active view horizontally"));
    actionSplitHorizontal->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_3));
    actionSplitHorizontal->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionSplitHorizontal, SIGNAL(triggered()), multiView_, SLOT(splitHorizontal()));



    ///////////////        SELECTION        ///////////////

    // Select All
    actionSelectAll = new QAction(tr("Select all"), this);
    actionSelectAll->setStatusTip(tr("Select all the objects."));
    actionSelectAll->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_A));
    actionSelectAll->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionSelectAll, SIGNAL(triggered()), scene_, SLOT(selectAll()));

    // Deselect All
    actionDeselectAll = new QAction(tr("Deselect all"), this);
    actionDeselectAll->setStatusTip(tr("Deselect all the objects."));
    actionDeselectAll->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_A));
    actionDeselectAll->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionDeselectAll, SIGNAL(triggered()), scene_, SLOT(deselectAll()));

    // Invert Selection
    actionInvertSelection = new QAction(tr("Invert Selection"), this);
    actionInvertSelection->setStatusTip(tr("Deselect all the selected objects and select all the other objects."));
    actionInvertSelection->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
    actionInvertSelection->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionInvertSelection, SIGNAL(triggered()), scene_, SLOT(invertSelection()));

    // Select Connected Objects
    actionSelectConnected = new QAction(tr("Select connected objects"), this);
    actionSelectConnected->setStatusTip(tr("Select all the objects that are connected to at least one selected object."));
    actionSelectConnected->setShortcut(Qt::Key_Tab);
    actionSelectConnected->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionSelectConnected, SIGNAL(triggered()), scene_, SLOT(selectConnected()));

    // Select Closure
    actionSelectClosure = new QAction(tr("Add boundary to selection"), this);
    actionSelectClosure->setStatusTip(tr("Add the boundary of the selected objects to the selection."));
    actionSelectClosure->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Tab));
    actionSelectClosure->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionSelectClosure, SIGNAL(triggered()), scene_, SLOT(selectClosure()));

    // Select Vertices
    actionSelectVertices = new QAction(tr("Select vertices"), this);
    actionSelectVertices->setStatusTip(tr("Deselect all the objects in the current selection other than vertices."));
    actionSelectVertices->setShortcut(QKeySequence(Qt::Key_S, Qt::Key_V));
    actionSelectVertices->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionSelectVertices, SIGNAL(triggered()), scene_, SLOT(selectVertices()));

    // Select Edges
    actionSelectEdges = new QAction(tr("Select edges"), this);
    actionSelectEdges->setStatusTip(tr("Deselect all the objects in the current selection other than edges."));
    actionSelectEdges->setShortcut(QKeySequence(Qt::Key_S, Qt::Key_E));
    actionSelectEdges->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionSelectEdges, SIGNAL(triggered()), scene_, SLOT(selectEdges()));

    // Select Faces
    actionSelectFaces = new QAction(tr("Select faces"), this);
    actionSelectFaces->setStatusTip(tr("Deselect all the objects in the current selection other than faces."));
    actionSelectFaces->setShortcut(QKeySequence(Qt::Key_S, Qt::Key_F));
    actionSelectFaces->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionSelectFaces, SIGNAL(triggered()), scene_, SLOT(selectFaces()));

    // Deselect Vertices
    actionDeselectVertices = new QAction(tr("Deselect vertices"), this);
    actionDeselectVertices->setStatusTip(tr("Deselect all vertices."));
    actionDeselectVertices->setShortcut(QKeySequence(Qt::Key_S, Qt::SHIFT + Qt::Key_V));
    actionDeselectVertices->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionDeselectVertices, SIGNAL(triggered()), scene_, SLOT(deselectVertices()));

    // Deselect Edges
    actionDeselectEdges = new QAction(tr("Deselect edges"), this);
    actionDeselectEdges->setStatusTip(tr("Deselect all edges."));
    actionDeselectEdges->setShortcut(QKeySequence(Qt::Key_S, Qt::SHIFT + Qt::Key_E));
    actionDeselectEdges->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionDeselectEdges, SIGNAL(triggered()), scene_, SLOT(deselectEdges()));

    // Deselect Faces
    actionDeselectFaces = new QAction(tr("Deselect faces"), this);
    actionDeselectFaces->setStatusTip(tr("Deselect all faces."));
    actionDeselectFaces->setShortcut(QKeySequence(Qt::Key_S, Qt::SHIFT + Qt::Key_F));
    actionDeselectFaces->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionDeselectFaces, SIGNAL(triggered()), scene_, SLOT(deselectFaces()));

    ///////////////        DEPTH        ///////////////

    // Raise
    actionRaise = new QAction(tr("Raise"), this);
    actionRaise->setStatusTip(tr("Raise the selected objects."));
    actionRaise->setShortcut(QKeySequence(Qt::Key_PageUp));
    actionRaise->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionRaise, SIGNAL(triggered()), scene_, SLOT(raise()));

    // Lower
    actionLower = new QAction(tr("Lower"), this);
    actionLower->setStatusTip(tr("Lower the selected objects."));
    actionLower->setShortcut(QKeySequence(Qt::Key_PageDown));
    actionLower->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionLower, SIGNAL(triggered()), scene_, SLOT(lower()));

    // Raise To Top
    actionRaiseToTop = new QAction(tr("Raise to top"), this);
    actionRaiseToTop->setStatusTip(tr("Raise the selected objects to the foreground."));
    actionRaiseToTop->setShortcut(QKeySequence(Qt::Key_Home));
    actionRaiseToTop->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionRaiseToTop, SIGNAL(triggered()), scene_, SLOT(raiseToTop()));

    // Lower To Bottom
    actionLowerToBottom = new QAction(tr("Lower to bottom"), this);
    actionLowerToBottom->setStatusTip(tr("Lower the selected objects to the background."));
    actionLowerToBottom->setShortcut(QKeySequence(Qt::Key_End));
    actionLowerToBottom->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionLowerToBottom, SIGNAL(triggered()), scene_, SLOT(lowerToBottom()));

    // Alternative Raise
    actionAltRaise = new QAction(tr("Alternative Raise"), this);
    actionAltRaise->setStatusTip(tr("Raise the selected objects, "
                                    "without enforcing that they stay below their boundary."));
    actionAltRaise->setShortcut(QKeySequence(Qt::ALT + Qt::Key_PageUp));
    actionAltRaise->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionAltRaise, SIGNAL(triggered()), scene_, SLOT(altRaise()));

    // Alternative Lower
    actionAltLower = new QAction(tr("Alternative Lower"), this);
    actionAltLower->setStatusTip(tr("Lower the selected objects, "
                                    "without enforcing that they stay below their boundary."));
    actionAltLower->setShortcut(QKeySequence(Qt::ALT + Qt::Key_PageDown));
    actionAltLower->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionAltLower, SIGNAL(triggered()), scene_, SLOT(altLower()));

    // Alternative Raise To Top
    actionAltRaiseToTop = new QAction(tr("Alternative Raise to top"), this);
    actionAltRaiseToTop->setStatusTip(tr("Raise the selected objects to the foreground, "
                                         "without enforcing that they stay below their boundary."));
    actionAltRaiseToTop->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Home));
    actionAltRaiseToTop->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionAltRaiseToTop, SIGNAL(triggered()), scene_, SLOT(altRaiseToTop()));

    // Alternative Lower To Bottom
    actionAltLowerToBottom = new QAction(tr("Alternative Lower to bottom"), this);
    actionAltLowerToBottom->setStatusTip(tr("Lower the selected objects to the background, "
                                            "without enforcing that they stay below their boundary."));
    actionAltLowerToBottom->setShortcut(QKeySequence(Qt::ALT + Qt::Key_End));
    actionAltLowerToBottom->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionAltLowerToBottom, SIGNAL(triggered()), scene_, SLOT(altLowerToBottom()));


    ///////////////        ANIMATION        ///////////////

    // Keyframe
    actionKeyframeSelection = new QAction(tr("Keyframe selection"), this);
    actionKeyframeSelection->setStatusTip(tr("Insert a key to all selected objects at current time."));
    actionKeyframeSelection->setShortcut(QKeySequence(Qt::Key_K));
    actionKeyframeSelection->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionKeyframeSelection, SIGNAL(triggered()), scene_, SLOT(keyframeSelection()));

    // Motion Paste
    actionMotionPaste = new QAction(tr("Motion paste"), this);
    actionMotionPaste->setStatusTip(tr("Paste the cells in the clipboard, and inbetween them with the copied cells."));
    actionMotionPaste->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_V));
    actionMotionPaste->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionMotionPaste, SIGNAL(triggered()), this, SLOT(motionPaste()));

    // Inbetween
    actionInbetweenSelection = new QAction(tr("Inbetween selection [Beta]"), this);
    actionInbetweenSelection->setStatusTip(tr("Automatically create inbetweens to interpolate the selection."));
    actionInbetweenSelection->setShortcut(QKeySequence(Qt::Key_I));
    actionInbetweenSelection->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionInbetweenSelection, SIGNAL(triggered()), scene_, SLOT(inbetweenSelection()));

    // Create inbetween Face
    actionCreateInbetweenFace = new QAction(tr("Create inbetween face [Beta]"), this);
    actionCreateInbetweenFace->setStatusTip(tr("Open the animated cycle editor to create a new inbetween face."));
    actionCreateInbetweenFace->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
    actionCreateInbetweenFace->setShortcutContext(Qt::ApplicationShortcut);
    connect(actionCreateInbetweenFace, SIGNAL(triggered()), this, SLOT(createInbetweenFace()));


    ///////////////        HELP        ///////////////

    // Online Documentation
    actionOnlineDocumentation = new QAction(tr("Online Documentation"), this);
    actionOnlineDocumentation->setStatusTip(tr("Redirects you to the online documentation of VPaint."));
    connect(actionOnlineDocumentation, SIGNAL(triggered()), this, SLOT(onlineDocumentation()));

    // Getting Started
    actionGettingStarted = new QAction(tr("Getting Started"), this);
    actionGettingStarted->setStatusTip(tr("First-time user? This is for you! Learn the basics of VPaint from scratch, in a few minutes."));
    connect(actionGettingStarted, SIGNAL(triggered()), this, SLOT(gettingStarted()));

    // Manual
    actionManual = new QAction(tr("User Manual"), this);
    actionManual->setStatusTip(tr("Learn every feature of VPaint."));
    connect(actionManual, SIGNAL(triggered()), this, SLOT(manual()));

    // About
    actionAbout = new QAction(tr("About VPaint"), this);
    actionAbout->setStatusTip(tr("Information about VPaint."));
    connect(actionAbout, SIGNAL(triggered()), this, SLOT(about()));
}



/*********************************************************************
 *                            Menus
 */


void MainWindow::createMenus()
{
    /// ---- FILE ----
    menuFile = new QMenu(tr("&File"));
    menuFile->addAction(actionNew);
    menuFile->addAction(actionOpen);
    menuFile->addSeparator();
    menuFile->addAction(actionSave);
    menuFile->addAction(actionSaveAs);
    menuFile->addSeparator();
    QMenu * exportMenu = menuFile->addMenu(tr("Export")); {
        exportMenu->addAction(actionExportPNG);
        exportMenu->addAction(actionExportSVG);
    }
    //menuFile->addSeparator();
    //menuFile->addAction(actionPreferences);
    menuFile->addSeparator();
    menuFile->addAction(actionQuit);
    menuBar()->addMenu(menuFile);


    /// ---- EDIT ----
    menuEdit = new QMenu(tr("&Edit"));
    menuEdit->addAction(actionUndo);
    menuEdit->addAction(actionRedo);
    menuEdit->addSeparator();
    menuEdit->addAction(actionCut);
    menuEdit->addAction(actionCopy);
    menuEdit->addAction(actionPaste);
    menuEdit->addSeparator();
    menuEdit->addAction(actionSmartDelete);
    menuEdit->addAction(actionHardDelete);
    //menuEdit->addAction(actionTest);
    menuBar()->addMenu(menuEdit);


    /// ---- VIEW ----
    menuView = new QMenu(tr("&View"));
    menuView->addAction(actionZoomIn);
    menuView->addAction(actionZoomOut);

    menuView->addSeparator();
    menuView->addAction(actionShowCanvas);
    menuView->addAction(actionEditCanvasSize);

    menuView->addSeparator();
    QMenu * displayModeMenu = menuView->addMenu(tr("Display Mode")); {
        displayModeMenu->addAction(actionDisplayModeNormal);
        displayModeMenu->addAction(actionDisplayModeNormalOutline);
        displayModeMenu->addAction(actionDisplayModeOutline);
    }
    menuView->addAction(actionOnionSkinning);

    menuView->addSeparator();
    menuView->addAction(actionSplitClose);
    menuView->addAction(actionSplitOne);
    menuView->addAction(actionSplitVertical);
    menuView->addAction(actionSplitHorizontal);

    menuView->addSeparator();
    menuView->addAction(global()->toolBar()->toggleViewAction());
    menuView->addAction(global()->toolModeToolBar()->toggleViewAction());
    menuView->addAction(dockTimeLine->toggleViewAction());
    menuView->addAction(dockBackgroundWidget->toggleViewAction());
    advancedViewMenu = menuView->addMenu(tr("Advanced [Beta]")); {
        advancedViewMenu->addAction(dockInspector->toggleViewAction());
        advancedViewMenu->addAction(dockAdvancedSettings->toggleViewAction());
        advancedViewMenu->addAction(dockAnimatedCycleEditor->toggleViewAction());
        advancedViewMenu->addAction(actionOpenClose3D);
        advancedViewMenu->addAction(actionOpenView3DSettings);
    }

    menuBar()->addMenu(menuView);

    /// ---- SELECTION ----
    menuSelection = new QMenu(tr("&Selection"));
    menuSelection->addAction(actionSelectAll);
    menuSelection->addAction(actionDeselectAll);
    menuSelection->addAction(actionInvertSelection);
    menuSelection->addSeparator();
    menuSelection->addAction(actionSelectConnected);
    menuSelection->addAction(actionSelectClosure);
    menuSelection->addSeparator();
    menuSelection->addAction(actionSelectVertices);
    menuSelection->addAction(actionSelectEdges);
    menuSelection->addAction(actionSelectFaces);
    menuSelection->addAction(actionDeselectVertices);
    menuSelection->addAction(actionDeselectEdges);
    menuSelection->addAction(actionDeselectFaces);
    menuBar()->addMenu(menuSelection);

    /// ---- DEPTH ----
    menuDepth = new QMenu(tr("&Depth"));
    menuDepth->addAction(actionRaise);
    menuDepth->addAction(actionLower);
    menuDepth->addAction(actionRaiseToTop);
    menuDepth->addAction(actionLowerToBottom);
    menuDepth->addSeparator();
    menuDepth->addAction(actionAltRaise);
    menuDepth->addAction(actionAltLower);
    menuDepth->addAction(actionAltRaiseToTop);
    menuDepth->addAction(actionAltLowerToBottom);
    menuBar()->addMenu(menuDepth);

    /// ---- ANIMATION ----
    menuAnimation = new QMenu(tr("&Animation"));
    menuAnimation->addAction(actionMotionPaste);
    menuAnimation->addAction(actionKeyframeSelection);
    menuAnimation->addAction(actionInbetweenSelection);
    menuAnimation->addAction(actionCreateInbetweenFace);
    menuBar()->addMenu(menuAnimation);

    /// ---- PLAYBACK ----
    menuPlayback = new QMenu(tr("&Playback"));
    menuPlayback->addAction(timeline()->actionGoToFirstFrame());
    menuPlayback->addAction(timeline()->actionGoToPreviousFrame());
    menuPlayback->addAction(timeline()->actionPlayPause());
    menuPlayback->addAction(timeline()->actionGoToNextFrame());
    menuPlayback->addAction(timeline()->actionGoToLastFrame());
    menuBar()->addMenu(menuPlayback);

    /// ---- HELP ----
    menuHelp = new QMenu(tr("&Help"));
    menuHelp->addAction(actionOnlineDocumentation);
    //menuHelp->addAction(actionGettingStarted);
    //menuHelp->addAction(actionManual);
    //menuHelp->addSeparator();
    menuHelp->addAction(actionAbout);
    //menuHelp->addAction(actionAboutQt);
    menuBar()->addMenu(menuHelp);
}









/*********************************************************************
 *               Dock Windows
 */

void MainWindow::createDocks()
{
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

    // ----- Settings ---------

    QScrollArea *dockSettings_scrollArea = new QScrollArea();
    dockSettings_scrollArea->setWidget(DevSettings::instance());
    dockSettings_scrollArea->setFrameShape(QFrame::NoFrame);
    dockSettings_scrollArea->setWidgetResizable(false);
    dockAdvancedSettings = new QDockWidget(tr("Advanced Settings [Beta]"));
    dockAdvancedSettings->setAllowedAreas(Qt::LeftDockWidgetArea |
                                  Qt::RightDockWidgetArea);
    dockAdvancedSettings->setWidget(dockSettings_scrollArea);
    addDockWidget(Qt::RightDockWidgetArea, dockAdvancedSettings);
    dockAdvancedSettings->hide();

    // ----- Object Properties ---------

    // Widget
    inspector = new ObjectPropertiesWidget();

    // Scroll area
    QScrollArea * dockObjectProperties_scrollArea = new QScrollArea();
    dockObjectProperties_scrollArea->setWidget(inspector);
    dockObjectProperties_scrollArea->setWidgetResizable(true);

    // Dock
    dockInspector = new QDockWidget(tr("Inspector [Beta]"));
    dockInspector->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dockInspector->setWidget(dockObjectProperties_scrollArea);
    addDockWidget(Qt::RightDockWidgetArea, dockInspector);
    dockInspector->hide();

    // Signal/Slot connection
    connect(scene(),SIGNAL(selectionChanged()),this,SLOT(updateObjectProperties()));

    // ----- Animated cycle editor ---------

    // Widget
    animatedCycleEditor = new AnimatedCycleWidget();

    // Dock
    dockAnimatedCycleEditor = new QDockWidget(tr("Animated Cycle Editor [Beta]"));
    dockAnimatedCycleEditor->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dockAnimatedCycleEditor->setWidget(animatedCycleEditor);
    addDockWidget(Qt::RightDockWidgetArea, dockAnimatedCycleEditor);
    dockAnimatedCycleEditor->hide();

    // ----- Background ---------

    // Widget
    backgroundWidget = new BackgroundWidget();
    backgroundWidget->setBackground(scene()->background());

    // Dock
    dockBackgroundWidget = new QDockWidget(tr("Background"));
    dockBackgroundWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dockBackgroundWidget->setWidget(backgroundWidget);
    addDockWidget(Qt::RightDockWidgetArea, dockBackgroundWidget);
    //dockBackgroundWidget->hide(); todo: uncomment (commented for convenience while developing)


    // ----- TimeLine -------------

    dockTimeLine = new QDockWidget(tr("Timeline"));
    dockTimeLine->setWidget(timeline_);
    dockTimeLine->setAllowedAreas(Qt::BottomDockWidgetArea);
    dockTimeLine->setFeatures(QDockWidget::DockWidgetClosable);
    dockTimeLine->setTitleBarWidget(new QWidget());
    addDockWidget(Qt::BottomDockWidgetArea, dockTimeLine);
}




/*********************************************************************
 *                          Status Bar
 */

void MainWindow::createStatusBar()
{
      //statusBar()->showMessage(tr("Hello! How are you doing today?"),2000);
}




/*********************************************************************
 *                           Toolbars
 */


void MainWindow::createToolbars()
{
    global()->createToolBars();
}

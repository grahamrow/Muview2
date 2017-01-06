#include <iostream>
#include <QtGui>
#include <QDir>
#include <QKeySequence>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QSignalMapper>
#include <QDebug>
#include <QTimer>
#include <QMap>
#include <QColorDialog>
#include <QGLFormat>
#include <QSharedPointer>
#include <QCommandLineParser>

#include "glwidget.h"
#include "window.h"
#include "ui_window.h"
#include "ui_preferences.h"
#include "qxtspanslider.h"

#include "OMFImport.h"
//#include "OMFHeader.h"

struct OMFImport;

Window::Window(QStringList arguments) :
    QMainWindow(NULL),
    ui(new Ui::Window)
{
    QCommandLineParser parser;
    parser.setApplicationDescription("3D Viewer of OVF/OMF Micromagnetic Data");
    parser.addHelpOption();
    parser.addVersionOption();

    // General input
    parser.addPositionalArgument("input", QCoreApplication::translate("Files or Directories", "Input files to open."));

    // Watch files
    QCommandLineOption watchDirectoryOption(QStringList() << "w" << "watch",
                QCoreApplication::translate("main", "Watch <directory> and update when new data appears therein."),
                QCoreApplication::translate("main", "directory"));
    parser.addOption(watchDirectoryOption);

    // Actually parse the arguments
    parser.process(arguments);
    const QStringList fileargs = parser.positionalArguments();
    QString watchDir = parser.value(watchDirectoryOption);

    // Initialize GUI
    ui->setupUi(this);

    // Specify an OpenGL format using the Core profile.
    // That is, no old-school fixed pipeline functionality
    QGLFormat glFormat;
#ifdef __APPLE__
    // This actually seems to yield a 4.1 context on Mavericks
    glFormat.setVersion(3, 2);
#else
    glFormat.setVersion( 3, 3 );
#endif
    glFormat.setProfile( QGLFormat::CoreProfile ); // Requires >=Qt-4.8.0
    glFormat.setSampleBuffers( true );

    // Create a GLWidget requesting our format
    viewport = new GLWidget( glFormat );
    ui->viewportHorizontalLayout->insertWidget(1,viewport, 1);

    // Clipboard
    clipboard = QApplication::clipboard();

    // File defaults
    lastSavedLocation = QDir::home();
    lastOpenedLocation = QDir::home();
    watcher = new QFileSystemWatcher;

	// Cache size
    cacheSize = 25;
	cachePos  = 0;

    // Sub-windows
	prefs = new Preferences(this);
    about = new AboutDialog(this);

    connect(prefs, SIGNAL(finished(int)), this, SLOT(updatePrefs()));

    initSlider(ui->xSlider);
    initSlider(ui->ySlider);
    initSlider(ui->zSlider);
    initSpanSlider(ui->xSpanSlider);
    initSpanSlider(ui->ySpanSlider);
    initSpanSlider(ui->zSpanSlider);

    // Rotation
    connect(ui->xSlider,  SIGNAL(valueChanged(int)),     viewport, SLOT(setXRotation(int)));
    connect(viewport, SIGNAL(xRotationChanged(int)), ui->xSlider, SLOT(setValue(int)));
    connect(ui->ySlider,  SIGNAL(valueChanged(int)),     viewport, SLOT(setYRotation(int)));
    connect(viewport, SIGNAL(yRotationChanged(int)), ui->ySlider, SLOT(setValue(int)));
    connect(ui->zSlider,  SIGNAL(valueChanged(int)),     viewport, SLOT(setZRotation(int)));
    connect(viewport, SIGNAL(zRotationChanged(int)), ui->zSlider, SLOT(setValue(int)));

    // Slicing
    connect(ui->xSpanSlider, SIGNAL(lowerValueChanged(int)), viewport, SLOT(setXSliceLow(int)));
    connect(ui->xSpanSlider, SIGNAL(upperValueChanged(int)), viewport, SLOT(setXSliceHigh(int)));
    connect(ui->ySpanSlider, SIGNAL(lowerValueChanged(int)), viewport, SLOT(setYSliceLow(int)));
    connect(ui->ySpanSlider, SIGNAL(upperValueChanged(int)), viewport, SLOT(setYSliceHigh(int)));
    connect(ui->zSpanSlider, SIGNAL(lowerValueChanged(int)), viewport, SLOT(setZSliceLow(int)));
    connect(ui->zSpanSlider, SIGNAL(upperValueChanged(int)), viewport, SLOT(setZSliceHigh(int)));

    // Animation
    ui->animSlider->setEnabled(false);

    // Actions
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(openAbout()));
    connect(ui->actionPreferences, SIGNAL(triggered()), this, SLOT(openSettings()));
    connect(ui->actionFiles, SIGNAL(triggered()), this, SLOT(openFiles()));
    connect(ui->actionDir, SIGNAL(triggered()), this, SLOT(openDir()));
    connect(ui->actionCopy, SIGNAL(triggered()), this, SLOT(copyImage()));

    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(saveImage()));
    connect(ui->actionSequence, SIGNAL(triggered()), this, SLOT(saveImageSequence()));

    connect(ui->actionCubes, SIGNAL(triggered()), this, SLOT(toggleDisplay()));
    connect(ui->actionCones, SIGNAL(triggered()), this, SLOT(toggleDisplay()));
    connect(ui->actionVectors, SIGNAL(triggered()), this, SLOT(toggleDisplay()));

    connect(ui->actionIncreaseSubsampling, SIGNAL(triggered()), viewport, SLOT(increaseSubsampling()));
    connect(ui->actionDecreaseSubsampling, SIGNAL(triggered()), viewport, SLOT(decreaseSubsampling()));

    displayType = new QActionGroup(this);
    displayType->addAction(ui->actionCubes);
    displayType->addAction(ui->actionCones);
    displayType->addAction(ui->actionVectors);
    ui->actionCubes->setChecked(true);

    signalMapper = new QSignalMapper(this);
    signalMapper->setMapping (ui->actionFollow, "") ;
    connect (signalMapper, SIGNAL(mapped(QString)), this, SLOT(watch(QString))) ;
    connect(ui->actionFollow, SIGNAL(triggered()), signalMapper, SLOT(map()));
    connect(ui->actionUnfollow, SIGNAL(triggered()), this, SLOT(stopWatch()));

    ui->xSlider->setValue(345 * 1600);
    ui->ySlider->setValue(0   * 1600);
    ui->zSlider->setValue(0   * 1600);
    setWindowTitle(tr("MuView 2.1.2"));

    // Data, don't connect until we are ready (probably still not ready here)...
    connect(ui->animSlider, SIGNAL(valueChanged(int)), this, SLOT(updateDisplayData(int)));

    // Watching a directory, if that is requested
    if (watchDir != "") {
        watch(watchDir);
    }

    // Otherwise we load files and directories
    if (watchDir == "" && fileargs.length() > 0) {
        foreach (QString item, fileargs) {
            QFileInfo info(item);
            if (!info.exists()) {
                qDebug() << "File " << item << " does not exist";
            } else {
                // Push our new content...
                if (info.isDir()) {
                    QDir chosenDir(item);
                    dirString = chosenDir.path()+"/";
                    QStringList filters;
                    filters << "*.omf" << "*.ovf";
                    chosenDir.setNameFilters(filters);
                    QStringList files = chosenDir.entryList();

                    foreach (QString file, files)
                    {
                        filenames << (dirString+file);
                        displayNames << (dirString+file);
                    }

                } else {
                    // just a normal file
                    filenames << (dirString+item);
                    displayNames << (dirString+item);
                }
            }
        }
        if (filenames.length() > 0) {
            processFilenames();
            gotoFrontOfCache();
            adjustAnimSlider(false);  // Refresh the animation bar
        } else {
            ui->statusbar->showMessage(QString("No omf or ovf filers"));
            qDebug() << "No omf or ovf filers found.";
        }
    }

    // This seems to solve the strange issue of the QGLwidget not filling its container...
    viewport->setFixedSize(QSize(800,600));
    this->adjustSize();
    viewport->setFixedSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);

    // Set preferences
    updatePrefs();
}

void Window::initSpanSlider(QxtSpanSlider *slider)
{
    slider->setRange(0 *16, 100 * 16);
    slider->setSingleStep(16);
    slider->setPageStep(15 * 16);
    slider->setTickInterval(15 * 16);
    slider->setTickPosition(QSlider::TicksRight);
    slider->setHandleMovementMode(QxtSpanSlider::NoOverlapping);
    slider->setLowerValue(0*16);
    slider->setUpperValue(100*16);
}

void Window::initSlider(QSlider *slider)
{
    slider->setRange(0, 360 * 1600);
    slider->setSingleStep(1600);
    slider->setPageStep(15 * 1600);
    slider->setTickInterval(15 * 1600);
    slider->setTickPosition(QSlider::TicksRight);
    slider->setValue(0);
}

void Window::adjustAnimSlider(bool back)
{
	int numFiles = filenames.size();
	if (numFiles > 1) {
        ui->animSlider->setRange(0, numFiles-1);
        ui->animSlider->setSingleStep(1);
        ui->animSlider->setPageStep(10);
        ui->animSlider->setTickInterval( numFiles < 20 ? numFiles : numFiles/20);
        ui->animSlider->setTickPosition(QSlider::TicksRight);
        ui->animSlider->setEnabled(true);
		if (back) {
            ui->animSlider->setSliderPosition(numFiles-1);
		} else {
            ui->animSlider->setSliderPosition(0);
		}
	} else {
        ui->animSlider->setEnabled(false);
	}
}

void Window::keyPressEvent(QKeyEvent *e)
{
    QWidget::keyPressEvent(e);
}

void Window::updatePrefs() {
    viewport->setBackgroundColor(prefs->getBackgroundColor());
    viewport->setSpriteDimensions(prefs->getSpriteResolution(), prefs->getVectorLength(), prefs->getVectorRadius(),
                                  prefs->getVectorTipToTail(), prefs->getVectorInnerToOuter(), prefs->getVectorOrigin());
    viewport->setBrightness(prefs->getBrightness());
    if (prefs->getImageDimensions() == QSize(-1,-1)) {
        viewport->setFixedSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
    } else {
        viewport->setFixedSize(prefs->getImageDimensions());
        this->adjustSize();
    }
    viewport->setColoredQuantity(prefs->getColorQuantity());
    viewport->setColorScale(prefs->getColorScale());
    viewport->setSpriteScale(prefs->getSpriteScale());
}

void Window::openSettings()
{
	prefs->exec();
}

void Window::openAbout()
{
    about->exec();
}

void Window::processFilenames() {
    // Looping over files
    for (int loadPos=0; loadPos<cacheSize && loadPos<filenames.size(); loadPos++) {
        // Attempt to read the file, null pointer returned if this fails
        QSharedPointer<OMFReader> omf = readOMF(filenames[loadPos]);
        if (omf.isNull()) {
            qDebug() << "Error loading file " << filenames[loadPos] << ", skipping...";
        }
        omfCache.push_back(omf);
    }
    cachePos = 0;
}

void Window::gotoFrontOfCache() {
    ui->statusbar->showMessage(filenames.front());
    viewport->updateData(omfCache.front());
    cachePos = 0;
    adjustAnimSlider(false); // Go to end of slider
}

void Window::gotoBackOfCache() {
    ui->statusbar->showMessage(filenames.back());
    viewport->updateData(omfCache.back());
    cachePos = filenames.size()-1;
    adjustAnimSlider(true); // Go to start of slider
}

void Window::clearCaches() {
    while (!omfCache.empty()) {
        omfCache.pop_back();
    }
}

void Window::openFiles()
{
    QStringList names;
    names = QFileDialog::getOpenFileNames(this,
        tr("Open File"), lastOpenedLocation.path(),
        tr("OVF Files (*.omf *.ovf)"));

    if (names.length() > 0)
    {
        lastOpenedLocation = QDir(names.at(0));
        filenames.clear();
        foreach(QString name, names) {
            if (name != "") {
                filenames.push_back(name);
                displayNames.push_back(name);
            }
        }
        clearCaches();
        processFilenames();
        gotoFrontOfCache();
    }
}

void Window::updateWatchedFiles() {
    // Look at all of the files in the directory
    // and add those which are not in the list of
    // original filenames

    // filenames contains the list of loaded files
    // watchedFiles is a map of files to load and their modification timestamps

    // When the timestamps in wathcedFiles stop changing we actually
    // push the relevant files into the OMF cache.

    QDir chosenDir(watchedDir);
    QString dirString = chosenDir.path()+"/";
    QStringList filters;
    filters << "*.omf" << "*.ovf";
    chosenDir.setNameFilters(filters);
    QStringList dirFiles = chosenDir.entryList();

    // compare to existing list of files
    bool changed = false;
    foreach(QString dirFile, dirFiles)
    {
        if (!filenames.contains(dirString + dirFile)) {
            // Haven't been loaded
            QString fullPath = dirString + dirFile;
            QFileInfo info(fullPath);

            if (!watchedFiles.contains(fullPath)) {
                // Not on the watch list yet
                watchedFiles.insert(fullPath, info.lastModified());
                // Callback this method to see if the file is done changing!
                QTimer::singleShot(100, this, SLOT(updateWatchedFiles()));
            } else {
                // on the watch list
                if (info.lastModified() == watchedFiles[fullPath]) {
                    // File size has stabalized
                    filenames.append(fullPath);
                    displayNames.append(dirFile);
                    changed = true;
                } else {
                    // File still changing
                    watchedFiles[fullPath] = info.lastModified();
                    // Callback this method to see if the file is done changing!
                    QTimer::singleShot(100, this, SLOT(updateWatchedFiles()));
                }
            }
        }
    }

    if (changed) {
        clearCaches(); // Blank slate
        QSharedPointer<OMFReader> omf = readOMF(filenames.last());
        if (omf.isNull()) {
            qDebug() << "Error loading file " << filenames.last() << ", skipping...";
        }
        omfCache.push_back(omf);
        gotoBackOfCache();
    }

}

void Window::updateDisplayData(int index)
{
    // Check to see if we've cached this data already.
    // Add and remove elements from the front and back
    // of the deque until we've caught up... if we're
    // too far out of range just scratch everything and
    // reload.
    if ( abs(index-cachePos) >= cacheSize ) {
            // Out of the realm of caching: clear the cache of pre-existing elements
        clearCaches();
        cachePos = index;
        for (int loadPos=index; loadPos<(index+cacheSize) && loadPos<filenames.size(); loadPos++) {
            // Attempt to read the file, null pointer returned if this fails
            QSharedPointer<OMFReader> omfdata = readOMF(filenames[loadPos]);
            if (omfdata.isNull()) {
                qDebug() << "Error loading file " << filenames[loadPos] << ", skipping...";
            }
            omfCache.push_back(omfdata);
        }
        cachePos = index;
    } else if ( index < cachePos ) {
            // Moving backwards, regroup for fast scrubbing!
        for (int loadPos=cachePos-1; loadPos >= index && loadPos<filenames.size(); loadPos--) {
            if (omfCache.size() == cacheSize) {
                omfCache.pop_back();
            }
            // Attempt to read the file, null pointer returned if this fails
            QSharedPointer<OMFReader> omfdata = readOMF(filenames[loadPos]);
            if (omfdata.isNull()) {
                qDebug() << "Error loading file " << filenames[loadPos] << ", skipping...";
            }
            omfCache.push_front(omfdata);
        }
        cachePos = index;
    }

    // We are within the current cache
    if (index < filenames.size()) {
        if (omfCache.at(index-cachePos).isNull()) {
            ui->statusbar->showMessage("File " + displayNames[index] + " was not understood by Muview and is being skipped.");
        } else {
            ui->statusbar->showMessage(displayNames[index]);
            // Update the Display
            viewport->updateData(omfCache.at(index-cachePos));
        }
    } else {
        ui->statusbar->showMessage(QString("Don't scroll so erratically..."));
    }
}

void Window::openDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
        lastOpenedLocation.path(),
        QFileDialog::ShowDirsOnly
        | QFileDialog::DontResolveSymlinks);
	
    if (dir != "")
    {
        QDir chosenDir(dir);
        lastOpenedLocation = chosenDir;
        dirString = chosenDir.path()+"/";
        QStringList filters;
        filters << "*.omf" << "*.ovf";
        chosenDir.setNameFilters(filters);
        QStringList dirFiles = chosenDir.entryList();
        if (dirFiles.length() == 0) {
            qDebug() << "No .omf or .ovf files were found in the specified directory.";
            ui->statusbar->showMessage("No .omf or .ovf files were found in the specified directory.");
        } else {
            filenames.clear();
            foreach (QString file, dirFiles) {
                filenames.push_back(dirString + file);
            }

            // persistent storage of filenames for top overlay
            displayNames = dirFiles;

            clearCaches();
            processFilenames();
            gotoFrontOfCache();
        }
    }
}

void Window::saveImageFile(QString name)
{
    if (name != "") {
        lastSavedLocation = QDir(name);
        QImage screen = viewport->grabFrameBuffer();
        screen.save(name, 0, 90); //format was (prefs->getFormat()).toStdString().c_str()
    }
}

void Window::copyImage()
{
    QImage screen = viewport->grabFrameBuffer();
    clipboard->setImage(screen);
}

void Window::saveImage()
{
    QString fileName, filter;
    QString format = prefs->getFormat();
    if (format == "JPG") {
        filter = tr("JPEG Images (*.jpg)");
    } else {
        filter = tr("PNG Images (*.png)");
    }
    fileName = QFileDialog::getSaveFileName(this, tr("Save Image"), lastSavedLocation.path(), tr("JPEG Images (*.jpg);;PNG Images (*.png)"), &filter);
    saveImageFile(fileName);
}

void Window::saveImageSequence()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Save Sequence to Directory"),
        lastSavedLocation.path(),
        QFileDialog::ShowDirsOnly
        | QFileDialog::DontResolveSymlinks);

    if (dir != "")
    {
        connect(viewport, SIGNAL(doneRenderingFrame(QString)), this, SLOT(saveImageFile(QString)));

        lastSavedLocation = QDir(dir);
        QImage screen;
        QString number;
        QString format = (prefs->getFormat()).toLower();
        QString outpath;
        for (int i=0; i<filenames.length(); i++) {
            number = QString("%1").arg(i, 6, 'd', 0, QChar('0'));
            ui->animSlider->setValue(i);
            outpath = dir+"/muviewSequence"+number+"."+format;
            ui->statusbar->showMessage("Saving file "+outpath);
            update();
            viewport->renderFrame(outpath);
        }

        disconnect(viewport, SIGNAL(doneRenderingFrame(QString)), this, SLOT(saveImageFile(QString)));
    }
}

void Window::watch(const QString& str)
{
    QString dir;
    // Don't show a dialog if we get this message from the command line
    if (str == "") {
        dir = QFileDialog::getExistingDirectory(this, tr("Watch Directory"),
            lastOpenedLocation.path(),
            QFileDialog::ShowDirsOnly
            | QFileDialog::DontResolveSymlinks);
    } else {
        dir = str;
    }

    if (filenames.length() > 0) {
        filenames.clear();
        displayNames.clear();
    }
    clearCaches();

    cachePos = 0; // reset position to beginning

    if (dir != "")
    {
        // Added the dir to the watch list
        if (watcher->directories().length() > 0) {
            watcher->removePaths(watcher->directories());
        }
        watcher->addPath(dir);
        watchedDir = dir;

        // Now read all of the current files
        QDir chosenDir(dir);
        lastOpenedLocation = chosenDir;
        QString dirString = chosenDir.path()+"/";
        QStringList filters;
        filters << "*.omf" << "*.ovf";
        chosenDir.setNameFilters(filters);
        QStringList dirFiles = chosenDir.entryList();

        if (dirFiles.length() == 0) {
            qDebug() << "No .omf or .ovf files were found in the specified directory. Waiting for input.";
            ui->statusbar->showMessage("No .omf or .ovf files were found in the specified directory. Waiting for input.");
        } else {
            // persistent storage of filenames for status bar
            displayNames = dirFiles;
            filenames.clear();
            foreach (QString file, dirFiles) {
                filenames.push_back(dirString + file);
            }

            processFilenames();
            gotoBackOfCache();
        }

        connect(watcher, SIGNAL(directoryChanged(QString)),
            this, SLOT(updateWatchedFiles()));
    }

}

void Window::stopWatch()
{
    if (watcher->directories().length() > 0) {
        watcher->removePaths(watcher->directories());
        disconnect(watcher, SIGNAL(directoryChanged(QString)),
                   this, SLOT(updateWatchedFiles(QString)));
    }
}

void Window::toggleDisplay() {
    if (ui->actionCubes->isChecked()) {
        viewport->toggleDisplay(0);
    } else if (ui->actionCones->isChecked()) {
        viewport->toggleDisplay(1);
    } else {
        viewport->toggleDisplay(2);
    }
}

Window::~Window()
{
    delete ui;
}


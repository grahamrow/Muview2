#include <deque>
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

#include "glwidget.h"
#include "window.h"
#include "ui_window.h"
#include "ui_preferences.h"
#include "qxtspanslider.h"

#include "OMFImport.h"
#include "OMFHeader.h"
#include "OMFContainer.h"

struct OMFImport;

Window::Window(int argc, char *argv[]) :
    QMainWindow(NULL),
    ui(new Ui::Window)
{
    ui->setupUi(this);

    // Clipboard
    clipboard = QApplication::clipboard();

    // File defaults
    lastSavedLocation = QDir::home();
    lastOpenedLocation = QDir::home();
    watcher = new QFileSystemWatcher;

	// Cache size
	cacheSize = 50;
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
    connect(ui->xSlider,  SIGNAL(valueChanged(int)),     ui->viewport, SLOT(setXRotation(int)));
    connect(ui->viewport, SIGNAL(xRotationChanged(int)), ui->xSlider, SLOT(setValue(int)));
    connect(ui->ySlider,  SIGNAL(valueChanged(int)),     ui->viewport, SLOT(setYRotation(int)));
    connect(ui->viewport, SIGNAL(yRotationChanged(int)), ui->ySlider, SLOT(setValue(int)));
    connect(ui->zSlider,  SIGNAL(valueChanged(int)),     ui->viewport, SLOT(setZRotation(int)));
    connect(ui->viewport, SIGNAL(zRotationChanged(int)), ui->zSlider, SLOT(setValue(int)));

    // Slicing
    connect(ui->xSpanSlider, SIGNAL(lowerValueChanged(int)), ui->viewport, SLOT(setXSliceLow(int)));
    connect(ui->xSpanSlider, SIGNAL(upperValueChanged(int)), ui->viewport, SLOT(setXSliceHigh(int)));
    connect(ui->ySpanSlider, SIGNAL(lowerValueChanged(int)), ui->viewport, SLOT(setYSliceLow(int)));
    connect(ui->ySpanSlider, SIGNAL(upperValueChanged(int)), ui->viewport, SLOT(setYSliceHigh(int)));
    connect(ui->zSpanSlider, SIGNAL(lowerValueChanged(int)), ui->viewport, SLOT(setZSliceLow(int)));
    connect(ui->zSpanSlider, SIGNAL(upperValueChanged(int)), ui->viewport, SLOT(setZSliceHigh(int)));

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

    connect(ui->actionCubes,   SIGNAL(triggered()), this, SLOT(toggleDisplay()));
    connect(ui->actionCones,   SIGNAL(triggered()), this, SLOT(toggleDisplay()));
    connect(ui->actionVectors, SIGNAL(triggered()), this, SLOT(toggleDisplay()));

    displayType = new QActionGroup(this);
    displayType->addAction(ui->actionCubes);
    displayType->addAction(ui->actionCones);
    displayType->addAction(ui->actionVectors);
    ui->actionCubes->setChecked(true);

    signalMapper = new QSignalMapper(this);
    signalMapper->setMapping (ui->actionFollow, "") ;
    connect (signalMapper, SIGNAL(mapped(QString)), this, SLOT(watchDir(QString))) ;
    connect(ui->actionFollow, SIGNAL(triggered()), signalMapper, SLOT(map()));
    connect(ui->actionUnfollow, SIGNAL(triggered()), this, SLOT(stopWatchingDir()));

    ui->xSlider->setValue(345 * 1600);
    ui->ySlider->setValue(0 * 1600);
    ui->zSlider->setValue(0 * 1600);
    setWindowTitle(tr("MuView 2.0b1"));

    // Data, don't connect until we are ready (probably still not ready here)...
    connect(ui->animSlider, SIGNAL(valueChanged(int)), this, SLOT(updateDisplayData(int)));

    // Load files from command line if supplied
    if (argc > 1) {
        QStringList rawList;
        for (int i=1; i<argc; i++) {
            rawList << argv[i];
        }

        if (rawList.contains(QString("-w"))) {
            if (rawList.indexOf("-w") < (rawList.length() - 1))  {
                watchDir(rawList[rawList.indexOf("-w")+1]);
            }
        } else {
            QStringList allLoadedFiles;
            foreach (QString item, rawList)
            {
                QFileInfo info(item);
                if (!info.exists()) {
                    std::cout << "File " << item.toStdString() << " does not exist" << std::endl;
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
                            displayNames << (dirString+item);
                        }

                    } else {
                        // just a normal file
                        filenames << (dirString+item);
                        displayNames << (dirString+item);
                    }
                }
            }

            processFilenames();
            gotoFrontOfCache();
            adjustAnimSlider(false);  // Refresh the animation bar
        }
    }

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
	//qDebug() << QString("Updating Animation Slider to size") << numFiles;
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
	//qDebug() << QString("Updated Animation Slider to size") << numFiles;
}

void Window::keyPressEvent(QKeyEvent *e)
{
    QWidget::keyPressEvent(e);
//    if (e->modifiers() == Qt::CTRL) {
//        // Close on Ctrl-Q or Ctrl-W
//        if (e->key() == Qt::Key_Q || e->key() == Qt::Key_W )
//            close();
//    } else {
//        QWidget::keyPressEvent(e);
//    }
}

void Window::updatePrefs() {
    ui->viewport->setBackgroundColor(prefs->getBackgroundColor());
    ui->viewport->setSpriteResolution(prefs->getSpriteResolution());
    ui->viewport->setBrightness(prefs->getBrightness());
    if (prefs->getImageDimensions() == QSize(-1,-1)) {
        ui->viewport->setFixedSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
    } else {
        ui->viewport->setFixedSize(prefs->getImageDimensions());
        this->adjustSize();
    }
    ui->viewport->setColoredQuantity(prefs->getColorQuantity());
    ui->viewport->setColorScale(prefs->getColorScale());
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
        header_ptr header = header_ptr(new OMFHeader());
        omfHeaderCache.push_back(header);
        omfCache.push_back(readOMF((filenames[loadPos]).toStdString(), *header));
    }
}

void Window::gotoFrontOfCache() {
    ui->viewport->updateHeader(omfHeaderCache.front(), omfCache.front());
    ui->statusbar->showMessage(filenames.front());
    ui->viewport->updateData(omfCache.front());
    cachePos = 0;
    adjustAnimSlider(false); // Go to end of slider
}

void Window::gotoBackOfCache() {
    ui->viewport->updateHeader(omfHeaderCache.back(), omfCache.back());
    ui->statusbar->showMessage(filenames.back());
    ui->viewport->updateData(omfCache.back());
    cachePos = filenames.size()-1;
    adjustAnimSlider(true); // Go to start of slider
}

void Window::clearCaches() {
    while (!omfHeaderCache.empty()) {
        omfHeaderCache.pop_back();
    }
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
//    qDebug() << names;

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

void Window::updateWatchedFiles(const QString& str) {
    // Look at all of the files in the directory
    // and add those which are not in the list of
    // original filenames

    // filenames contains the list of loaded files
    // watchedFiles is a map of files to load and their modification timestamps

    // When the timestamps in wathcedFiles stop changing we actually
    // push the relevant files into the OMF cache.

    QDir chosenDir(str);
    QString dirString = chosenDir.path()+"/";
    QStringList filters;
    filters << "*.omf" << "*.ovf";
    chosenDir.setNameFilters(filters);
    QStringList dirFiles = chosenDir.entryList();

    OMFHeader tempHeader = OMFHeader();

    // compare to existing list of files
    bool changed = false;
    foreach(QString dirFile, dirFiles)
    {
        //qDebug() << QString("Changed File!") << dirFile;
        if (!filenames.contains(dirString + dirFile)) {
            // Haven't been loaded
            QString fullPath = dirString + dirFile;
            QFileInfo info(fullPath);
            //qDebug() << QString("Not in filenames: ") << (dirFile);

            if (!watchedFiles.contains(fullPath)) {
                // Not on the watch list yet
                watchedFiles.insert(fullPath, info.lastModified());
                //qDebug() << QString("Inserted: ") << (dirFile);
            } else {
                // on the watch list
                if (info.lastModified() == watchedFiles[fullPath]) {
                    // File size has stabalized
                    //qDebug() << QString("Stable, pushed") << dirFile;
                    filenames.append(fullPath);
                    displayNames.append(dirFile);
                    //omfCache.push_back(readOMF(fullPath.toStdString(), tempHeader));
                    changed = true;
                } else {
                    // File still changing
                    //qDebug() << QSt        //qDebug() << QString("Pushing Back") << filenames[loadPos];ring("Unstable") << dirFile;
                    watchedFiles[fullPath] = info.lastModified();
                }
            }
        }
    }

    if (changed) {
        clearCaches();
        processFilenames();
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
            // Out of the realm of caching
            // Clear the cache of pre-existing elements
            //qDebug() << QString("Clearing the cache, too far out of range") << index << cachePos;
        clearCaches();
        cachePos = index;
        for (int loadPos=index; loadPos<(index+cacheSize) && loadPos<filenames.size(); loadPos++) {
            header_ptr header = header_ptr(new OMFHeader());
            omfHeaderCache.push_back(header);
            omfCache.push_back(readOMF((filenames[loadPos]).toStdString(), *header));
        }
        cachePos = index;
    } else if ( index < cachePos ) {
            // Moving backwards, regroup for fast scrubbing!
            //qDebug() << QString("Moving backwards") << index << cachePos;
        for (int loadPos=cachePos-1; loadPos >= index && loadPos<filenames.size(); loadPos--) {
            if (omfCache.size()==uint(cacheSize)) {
                omfCache.pop_back();
                omfHeaderCache.pop_back();
            }
            header_ptr header = header_ptr(new OMFHeader());
            omfHeaderCache.push_front(header);
            omfCache.push_front(readOMF((filenames[loadPos]).toStdString(), *header));
			
        }
        cachePos = index;
    }

    // We are within the current cache
    if (index < filenames.size()) {
        //qDebug() << QString("In Cache Range") << index << cachePos;
        ui->statusbar->showMessage(displayNames[index]);
        // Update the Display
        //qDebug() << QString("Current cache size") << omfCache.size();
        ui->viewport->updateHeader(omfHeaderCache.at(index-cachePos), omfCache.at(index-cachePos));
        ui->viewport->updateData(omfCache.at(index-cachePos));
    } else {
            //qDebug() << QString("Out of Cache Range!!!!") << index << cachePos;
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

void Window::saveImageFile(QString name)
{
    if (name != "") {
//        qDebug() << "Image saving triggered" << name;
        lastSavedLocation = QDir(name);
        QImage screen = ui->viewport->grabFrameBuffer();
        screen.save(name, (prefs->getFormat()).toStdString().c_str(), 90);
    }
}

void Window::copyImage()
{
    QImage screen = ui->viewport->grabFrameBuffer();
    clipboard->setImage(screen);
}

void Window::saveImage()
{
    QString fileName;
    fileName = QFileDialog::getSaveFileName(this, tr("Save Image"), lastSavedLocation.path());
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
        connect(ui->viewport, SIGNAL(doneRenderingFrame(QString)), this, SLOT(saveImageFile(QString)));

        lastSavedLocation = QDir(dir);
        QImage screen;
        QString number;
        for (int i=0; i<filenames.length(); i++) {
            number = QString("%1").arg(i, 6, 'd', 0, QChar('0'));
            ui->viewport->renderFrame(dir+"/muviewSequence"+number+".jpg");
            ui->animSlider->setValue(i);
            ui->statusbar->showMessage("Saving file"+dir+"/muviewSequence"+number+".jpg");
            update();
        }

        disconnect(ui->viewport, SIGNAL(doneRenderingFrame(QString)), this, SLOT(saveImageFile(QString)));
    }
}

void Window::watchDir(const QString& str)
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

    filenames.clear();
    displayNames.clear();
    clearCaches();

    cachePos = 0; // reset position to beginning

    if (dir != "")
    {
        // Added the dir to the watch list
        watcher->removePaths(watcher->directories());
        watcher->addPath(dir);

        // Now read all of the current files
        QDir chosenDir(dir);
        lastOpenedLocation = chosenDir;
        QString dirString = chosenDir.path()+"/";
        QStringList filters;
        filters << "*.omf" << "*.ovf";
        chosenDir.setNameFilters(filters);
        QStringList dirFiles = chosenDir.entryList();

        // persistent storage of filenames for status bar
        displayNames = dirFiles;

        foreach (QString file, dirFiles) {
            filenames.push_back(dirString + file);
        }

        processFilenames();
        gotoFrontOfCache();
//        if (filenames.length()>0) {

//            // Only cache the last file
//            header_ptr header = header_ptr(new OMFHeader());
//            cachePos = filenames.size()-1;
//            omfHeaderCache.push_back(header);
//            omfCache.push_back(readOMF(filenames.last().toStdString(), *header));

//            // Update the Display with the first element
//            ui->viewport->updateHeader(omfHeaderCache.back(), omfCache.back());
//            ui->viewport->updateData(omfCache.back());
			
//            // Update the top overlay
//            ui->statusbar->showMessage(displayNames.back());

//            // Refresh the animation bar
//            adjustAnimSlider(true);
//        }
        // Now the callbacks
        connect(watcher, SIGNAL(directoryChanged(QString)),
            this, SLOT(updateWatchedFiles(QString)));
    }

}

void Window::stopWatchingDir()
{
    if (watcher->directories().length() > 0) {
        watcher->removePaths(watcher->directories());
        disconnect(watcher, SIGNAL(directoryChanged(QString)),
                   this, SLOT(updateWatchedFiles(QString)));
    }
}

void Window::toggleDisplay() {
    if (ui->actionCubes->isChecked()) {
        ui->viewport->toggleDisplay(0);
    } else if (ui->actionCones->isChecked()) {
        ui->viewport->toggleDisplay(1);
    } else {
        ui->viewport->toggleDisplay(2);
    }
}

Window::~Window()
{
    delete ui;
}


#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QMap>
#include <QString>
#include <QDateTime>
#include <QDir>
#include <boost/smart_ptr.hpp>

// Other parts of the interface
#include "preferences.h"
#include "aboutdialog.h"

// For reading OMF files
#include "OMFContainer.h"
#include "OMFHeader.h"
#include <deque>

typedef boost::shared_ptr<OMFHeader> header_ptr;

// Forward Declarations
class QSlider;
class QxtSpanSlider;
class QSignalMapper;
class QGLFunctions;
class QActionGroup;
class QFileSystemWatcher;

namespace Ui {
    class Window;
}

class Window : public QMainWindow
{
    Q_OBJECT

public:
    explicit Window(int argc, char *argv[]);
    ~Window();
    QSize sizeHint();
protected:
    void keyPressEvent( QKeyEvent* e );

private slots:
    void openFiles();
    void openDir();
    void saveImage();
    void saveImageSequence();
    void saveImageFile(QString name);
    void copyImage();
    void watchDir(const QString& str);
    void stopWatchingDir();
    void toggleDisplay();
    void updateWatchedFiles(const QString& str);
    void openSettings();
    void openAbout();
    void updateDisplayData(int index);
    void updatePrefs();

private:
    Ui::Window *ui;
    QActionGroup *displayType;
    Preferences *prefs;
    AboutDialog *about;
    QClipboard *clipboard;

    // Convenience Functions for Sliders
    void initSlider(QSlider *slider);
    void initSpanSlider(QxtSpanSlider *slider);
    void adjustAnimSlider(bool back);

    // ============================================================
    // Storage and caching
    // A finite number of files, as governed by cacheSize, will
    // reside in memory at a given time, otherwise we will choke
    // the system on large output directories.
    // ============================================================

    int cacheSize; // Maxmimum cache size
    int cachePos;  // Current location w.r.t list of all filenames

    void clearCaches();
    void gotoBackOfCache();
    void gotoFrontOfCache();
    void processFilenames();

    std::deque<array_ptr>  omfCache;
    std::deque<header_ptr> omfHeaderCache;
    QStringList filenames;
    QStringList displayNames;

    QString dirString;
    QDir lastSavedLocation;
    QDir lastOpenedLocation;

    // Watch dirs
    QSignalMapper* signalMapper;
    QFileSystemWatcher *watcher;
    QMap<QString, QDateTime> watchedFiles;
    bool noFollowUpdate;

};

#endif

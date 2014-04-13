#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QMap>
#include <QString>
#include <QDateTime>
#include <QDir>
#include <QSharedPointer>
#include <QVector>

// Other parts of the interface
#include "preferences.h"
#include "aboutdialog.h"

// For reading OMF files
#include "OMFHeader.h"
#include "matrix.h"
#include "glwidget.h"

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
    explicit Window(QStringList arguments);
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
    void watch(const QString& str);
    void stopWatch();
    void toggleDisplay();
//    void updateWatchedFiles(const QString& str);
    void updateWatchedFiles();
    void openSettings();
    void openAbout();
    void updateDisplayData(int index);
    void updatePrefs();

private:
    Ui::Window *ui;
    GLWidget *viewport;
    QActionGroup *displayType;
    Preferences *prefs;
    AboutDialog *about;
    QClipboard *clipboard;

    // Convenience Functions for Sliders
    void initSlider(QSlider *slider);
    void initSpanSlider(QxtSpanSlider *slider);
    void adjustAnimSlider(bool back);

    // ============================================================
    // Storage and caching:
    //
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

    QVector<QSharedPointer<matrix> >    omfCache;
    QVector<QSharedPointer<OMFHeader> > omfHeaderCache;
    QStringList filenames;
    QStringList displayNames;

    QString dirString;
    QDir lastSavedLocation;
    QDir lastOpenedLocation;

    // Watch dirs
    QSignalMapper* signalMapper;
    QFileSystemWatcher *watcher;
    QMap<QString, QDateTime> watchedFiles;
    QString watchedDir;
    bool noFollowUpdate;

};

#endif

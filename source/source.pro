QT          += core gui widgets opengl
TEMPLATE     = app
TARGET       = ../muview
INCLUDEPATH += $$top_srcdir
UI_DIR      = uics
MOC_DIR     = mocs
OBJECTS_DIR = objs

CONFIG += release

linux {
    VERSION = 2.1.2
    #CONFIG += static
    message(Building in Linux Environment)
    message("    will install muview (binary) to /usr/local/bin")
    target.path = /usr/local/bin

    UNAME = $$system(uname -a)

    contains(UNAME, .*Ubuntu): UBUNTU = 1

    contains(UBUNTU, 1): {
        message("Ubuntu type distribution found:")
        message("   will install muview.desktop to /usr/share/applications")
        message("   will install icons to /usr/share/icons/hicolor/")
	    desktopfile.path = /usr/share/applications
	    desktopfile.files += resources/muview.desktop
        iconfile512.path  = /usr/share/icons/hicolor/512x512/apps
        iconfile512.files = resources/512x512/muview.png
        iconfile256.path  = /usr/share/icons/hicolor/256x256/apps
        iconfile256.files = resources/256x256/muview.png
        iconfile128.path  = /usr/share/icons/hicolor/128x128/apps
        iconfile128.files = resources/128x128/muview.png
        iconfile64.path   = /usr/share/icons/hicolor/64x64/apps
        iconfile64.files  = resources/64x64/muview.png
        iconfile32.path   = /usr/share/icons/hicolor/32x32/apps
        iconfile32.files  = resources/32x32/muview.png
        iconfile16.path   = /usr/share/icons/hicolor/16x16/apps
        iconfile16.files  = resources/16x16/muview.png

        INSTALLS += desktopfile \
                    iconfile512 iconfile256 iconfile128 \
                    iconfile64 iconfile32 iconfile16
    }

    pixmap.path  = /usr/share/pixmaps
    pixmap.files = resources/256x256/muview.png

    INSTALLS += target  pixmap
}

macx {
    message(Building in Mac OSX Environment)
    message("    will install muview (app) to /Applications")
    target.path = /Applications

    VERSION   = 2.1.2
    TARGET    = ../Muview
    ICON      = resources/muview.icns
    CONFIG   += dead-strip
    INSTALLS += target
}

win32 {
    message(Building in Windows Environment)
    message("    will install muview (binary) to C:\\Program Files")
    target.path = "C:\\Program Files"

    INSTALLS += target
}

SOURCES +=  \
    matrix.cpp \
    main.cpp \
    window.cpp \
    glwidget.cpp \
    glwidget_input.cpp \
    glwidget_assets.cpp \
    qxtspanslider.cpp \
    preferences.cpp \
    aboutdialog.cpp \
    OMFImport.cpp


HEADERS  += \
    matrix.h \
    glwidget.h \
    qxtspanslider.h \
    qxtspanslider_p.h \
    preferences.h \
    aboutdialog.h \
    window.h \
    OMFEndian.h \
    OMFImport.h

FORMS += \
    preferences.ui \
    window.ui \
    aboutdialog.ui

OTHER_FILES +=  \
    shaders/diffuse.frag \
    shaders/standard.frag \
    shaders/standard.vert \
    resources/splash.png \
    resources/splash2.png \
    resources/muview.desktop \
    resources/512x512/muview.png \
    resources/256x256/muview.png \
    resources/128x128/muview.png \
    resources/64x64/muview.png \
    resources/32x32/muview.png \
    resources/16x16/muview.png \
    shaders/diffuseMacOSX.frag \
    shaders/diffuse42.frag \
    shaders/diffuse42.vert \
    shaders/standard42.frag \
    shaders/standard42.vert

RESOURCES += \
    resources.qrc


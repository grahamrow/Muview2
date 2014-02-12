
QT           += core gui opengl
LIBS         += libomf/libomf.a
INCLUDEPATH  += $$PWD/libomf
TARGET       = muview
TEMPLATE     = app

linux {
    message(Building in Linux Environment)
    message("    will install muview (binary) to /usr/local/bin")
    target.path = /usr/local/bin

    UNAME = $$system(uname -a)

    contains(UNAME, .*Ubuntu): UBUNTU = 1

    contains(UBUNTU, 1): {
        message("Ubuntu type distribution found:")
        message("   will install muview.desktop to /usr/share/applications")
	desktopfile.path = /usr/share/applications
	desktopfile.files += resources/muview.desktop
	INSTALLS += desktopfile
    }

    INSTALLS += target
}

macosx {
    message(Building in Mac OS X Environment)
    message("    will install muview (binary) to /Applications")
    message("    with link at /usr/local/bin")
    target.path = /Applications

    INSTALLS += target
}

windows {
    message(Building in Windows Environment)
    message("    will install muview (binary) to C:\\Program Files")
    target.path = "C:\\Program Files"

    INSTALL += target
}

SOURCES +=  \
    main.cpp \
    window.cpp \
    glwidget.cpp \
    glwidget_input.cpp \
    glwidget_assets.cpp \
    qxtspanslider.cpp \
    preferences.cpp \
    aboutdialog.cpp \
    analysis.cpp

HEADERS  += \
    glwidget.h \
    qxtspanslider.h \
    qxtspanslider_p.h \
    preferences.h \
    aboutdialog.h \
    window.h \
    analysis.h

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
    resources/muview.desktop

RESOURCES += \
    resources.qrc


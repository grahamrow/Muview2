
QT           += core gui opengl
LIBS         += -L$$PWD/libomf -lomf
INCLUDEPATH  += $$PWD/libomf
TARGET       = muview
TEMPLATE     = app

linux {
    message(Building in Linux Environment)
    message("    will installed muview (binary) to /usr/local/bin")
    target.path = /usr/local/bin

    UNAME = $$system(uname -a)

    contains(UNAME, .*Ubuntu): UBUNTU = 1

    contains(UBUNTU, 1): {
        message("Ubuntu type distribution found:")
        message("   will install muview.desktop to /usr/share/applications")
    }

    INSTALLS += target
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


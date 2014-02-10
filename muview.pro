
QT          += core gui opengl
LIBS        += -L$$PWD/libomf -lomf
INCLUDEPATH += $$PWD/libomf
TARGET       = muview
TEMPLATE     = app

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
    shaders/standard.vert

RESOURCES += \
    resources.qrc


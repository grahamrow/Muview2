TARGET   = omf
TEMPLATE = lib
CONFIG += staticlib
HEADERS = OMFImport.h \
	  OMFContainer.h \
          OMFHeader.h \
	  OMFEndian.h

SOURCES = OMFImport.cpp \
	  OMFHeader.cpp


macx: {
	message("Running on Mac OSX:")
	LIBS += -L/usr/local/opt/qt5/lib \
			  -L/usr/local/Cellar/boost/1.55.0/lib
	INCLUDEPATH += /usr/local/opt/qt5/include \
					 /usr/local/Cellar/boost/1.55.0/include
}

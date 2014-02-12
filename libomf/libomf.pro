TARGET   = omf
TEMPLATE = lib
CONFIG += staticlib
HEADERS = OMFImport.h \
	  OMFContainer.h \
          OMFHeader.h \
	  OMFEndian.h

SOURCES = OMFImport.cpp \
	  OMFHeader.cpp


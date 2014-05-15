#!/bin/bash
# Do this through a MINGW terminal to preserve sanity...
# Compile in QtCreator first...

VERSION="2.1.1"
NAME="Muview${VERSION}"
TARGET="muview.exe"
MANIFEST="muview.exe.embed.manifest"
BUILD_DIR="../build-muview-Desktop_Qt_5_2_1_MSVC2012_OpenGL_64bit-Release/source"
SOURCE_DIR="/C/Qt/Qt5.2.1/5.2.1/msvc2012_64_opengl/bin"
QTDLLS="Qt5Core Qt5Gui Qt5Widgets Qt5OpenGL icuin51 icuuc51 icudt51"
WINDLL_DIR="/C/Windows/System32"
WINDLLS="msvcp110 msvcr110"
PLATFORMDLLS="qwindows qminimal"
PLUGIN_DIR="C:\Qt\Qt5.2.1\5.2.1\msvc2012_64_opengl\plugins"
FILES="README.md LICENSE"

# Kill old product
if [ -d $NAME ]
  then
  echo "Removing $NAME"
  rm -r $NAME
fi

mkdir -p $NAME/platforms

for DLL in $QTDLLS
do
	echo "Copying ${DLL}.dll"
	cp "${SOURCE_DIR}/${DLL}.dll" ${NAME}
done

for DLL in $WINDLLS
do
	echo "Copying ${DLL}.dll"
	cp "${WINDLL_DIR}/${DLL}.dll" ${NAME}
done

for DLL in $PLATFORMDLLS
do
	echo "Copying ${DLL}.dll"
	cp "${PLUGIN_DIR}/platforms/${DLL}.dll" ${NAME}/platforms
done

cp "${BUILD_DIR}/${TARGET}" "${BUILD_DIR}/${MANIFEST}" $NAME
echo "Done deploying!"

#!/bin/bash

# Static build of Qt5.2.1 done with the following commands
# ./configure -release -opensource -static -opengl desktop -fontconfig -qt-xkbcommon -qt-xcb
# make -j10 sub-src

VERSION="2.0"
NAME="Muview${VERSION}"

if [ -e muview ]
	then
	rm muview
fi
if [ -e "${NAME}.tar.bz" ]
	then
	rm "${NAME}.tar.bz"
fi
# Kill old product
if [ -d $NAME ]
  then
  echo "Removing $applicationName"
  rm -r $NAME
fi

cd source
make clean
qmake
cd ..
make clean
qmake
make -j20

FILES="muview README.md LICENSE"

mkdir $NAME
# Copy the install-only makefile to deploy directory
cp deploy/MakefileInstall $NAME/Makefile
# Copy the necessary resources, delete extraneous ones
cp -R source/resources $NAME
rm $NAME/resources/muview.icns $NAME/resources/splash*.png
# Copy the rest of the files
cp $FILES $NAME

tar -czvf "${NAME}.tar.gz" $NAME
echo "Done deploying!"

#!/bin/bash

NAME="Muview"

qmake -r -spec linux-g++ -config release
make -j4

FILES="muview README.md LICENSE"

mkdir $NAME
# Copy the install-only makefile to deploy directory
cp deploy/MakefileInstall Muview/Makefile
# Copy the necessary resources, delete extraneous ones
cp -R source/resources $NAME
rm $NAME/resources/muview.icns $NAME/resources/splash*.png
# Copy the rest of the files
cp $FILES $NAME

tar -czvf "Muview.tar.gz" $NAME
echo "Done deploying on Linux!"

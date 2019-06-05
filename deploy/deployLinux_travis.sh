#!/bin/bash

NAME="Muview"

# Fix version numbers
if [ "$TRAVIS" == true ]
  then
  echo "Updating version numbers"
  for name in source/aboutdialog.ui source/source.pro source/window.cpp
  do
    sed "s|2.2|${TRAVIS_BRANCH}|" $name > $name.temp
    mv $name.temp $name
  done
fi

qmake -r -spec linux-g++ -config release
make -j4

FILES="muview README.md LICENSE"

mkdir $NAME
mkdir deploy/artifacts
# Copy the install-only makefile to deploy directory
cp deploy/MakefileInstall Muview/Makefile
# Copy the necessary resources, delete extraneous ones
cp -R source/resources $NAME
rm $NAME/resources/muview.icns $NAME/resources/splash*.png
# Copy the rest of the files
cp $FILES $NAME

# Package using linuxdeployqt
# echo "Creating AppImage bundle at $PWD/Muview.AppImage"
echo "using qmake from $(which qmake)"
echo "using linuxdeployqt from $(which linuxdeployqt)"

cd $NAME
linuxdeployqt muview -bundle-non-qt-libs -verbose=1
linuxdeployqt resources/muview.desktop -appimage -verbose=1
mv Muview_Viewer*x86_64.AppImage ../deploy/artifacts/Muview2-x86_64.AppImage
rm AppRun
cd ..

tar -czvf "Muview.tar.gz" $NAME
mv Muview.tar.gz deploy/artifacts
echo "Done deploying on Linux!"

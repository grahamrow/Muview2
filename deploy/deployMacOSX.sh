#!/bin/bash

buildDir='./'
applicationName="Muview.app"
finalDMGName="Muview.dmg"
title="Muview"

# Kill old product
if [ -d $applicationName ]
  then
  echo "Removing $applicationName"
  rm -r $applicationName
fi
if [ -d $buildDir/$applicationName ]
  then
  echo "Removing $buildDir/$applicationName"
  rm -r $buildDir/$applicationName
fi

# This produces Muview 
cd source
make clean
qmake -config release
cd ..
make clean
qmake -config release
make -j4

echo "Creating Bundle at $PWD/Muview.app"
echo "using qmake from $(which qmake)"
echo "using macdeployqt from $(which macdeployqt)"

macdeployqt $buildDir/$applicationName -verbose=3

if [ -e tmp.dmg ]
	then
	rm tmp.dmg
fi

if [ -e $finalDMGName ]
	then
	rm $finalDMGName
fi

echo "Creating Staging Directory"
rm -rf staging
mkdir -p staging/.background
cp deploy/background-dmg.png staging/.background
cp Muview.DS_Store staging/.DS_Store
cp -R $applicationName staging

echo "Creating DMG"
hdiutil makehybrid -hfs -hfs-volume-name $title -hfs-openfolder staging staging -o tmp.dmg
hdiutil convert -format UDZO tmp.dmg -o $finalDMGName


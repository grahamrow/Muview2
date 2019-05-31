#!/bin/bash

buildDir='./'
applicationName="Muview.app"
finalDMGName="Muview.dmg"
title="Muview"

# Fix version numbers
for name in source/aboutdialog.ui source/source.pro source/window.cpp
do
  sed "s/2.2/${TRAVIS_BRANCH}/" $name > $name.temp
  mv $name.temp $name
done

# qmake version
qmake_vers=$(qmake -v | tail -n1 | sed -n 's|.*\(/usr/.*\)/lib|\1|p')

# This produces Muview 
qmake -config release
make -j4

echo "Creating Bundle at $PWD/Muview.app"
echo "using qmake from $(which qmake)"
echo "using macdeployqt from $(which macdeployqt)"

# Macdeployqt has known problems for brewed qt
macdeployqt $buildDir/$applicationName -verbose=3 -always-overwrite
curl -o /tmp/macdeployqtfix.py https://raw.githubusercontent.com/aurelien-rainone/macdeployqtfix/master/macdeployqtfix.py
python /tmp/macdeployqtfix.py Muview.app/Contents/MacOS/Muview $qmake_vers

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

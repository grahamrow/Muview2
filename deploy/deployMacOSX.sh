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

# qmake version
qmake_vers=$(qmake -v | tail -n1 | sed -n 's|.*\(/usr/.*\)/lib|\1|p')

# This produces Muview 
cd source
qmake -config release
make clean
cd ..
qmake -config release
make clean
make -j4

echo "Creating Bundle at $PWD/Muview.app"
echo "using qmake from $(which qmake)"
echo "using macdeployqt from $(which macdeployqt)"

# Macdeployqt has known problems for brewed qt
macdeployqt $buildDir/$applicationName -verbose=3 -always-overwrite
curl -o fix_temp.py https://raw.githubusercontent.com/aurelien-rainone/macdeployqtfix/master/macdeployqtfix.py
sed "s/deps = \[s.strip/deps = \[s.decode\(\).strip/" fix_temp.py > fix.py
python fix.py Muview.app/Contents/MacOS/Muview $qmake_vers
rm fix_temp.py fix.py

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
mkdir deploy
mv $finalDMGName deploy

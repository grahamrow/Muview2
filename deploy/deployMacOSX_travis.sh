#!/bin/bash

buildDir='./'
backgroundPictureName="deploy/background-dmg.png"
applicationName="Muview.app"
finalDMGName="Muview.dmg"
title="Muview"

# Fix version numbers
for name in source/aboutdialog.ui source/source.pro source/window.cpp
do
  sed "s/2.1.2/${TRAVIS_BRANCH}/" $name > $name.temp
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

echo "Creating disk image"
hdiutil create pack.temp.dmg -srcfolder $buildDir/$applicationName -format UDRW -volname $title

echo "Mounting disk image"
hdiutil attach -readwrite -noverify -noautoopen "pack.temp.dmg" -mountpoint mnt
sleep 1

mkdir "mnt/.background"
cp deploy/background-dmg.png "mnt/.background"

echo '
   tell application "Finder"
     tell disk "mnt"
           open
           set current view of container window to icon view
           set toolbar visible of container window to false
           set statusbar visible of container window to false
           set the bounds of container window to {400, 100, 1071, 632}
           set theViewOptions to the icon view options of container window
           set arrangement of theViewOptions to not arranged
           set icon size of theViewOptions to 350
           set background picture of theViewOptions to file ".background:background-dmg.png"
           set position of item "'${applicationName}'" of container window to {212, 282}
           update without registering applications
           delay 5
           eject
     end tell
   end tell
' | osascript


sync
hdiutil convert "pack.temp.dmg" -format UDZO -imagekey zlib-level=9 -o "${finalDMGName}"
rm -f pack.temp.dmg 

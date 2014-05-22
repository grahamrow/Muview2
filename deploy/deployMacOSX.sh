#!/bin/bash

#buildDir="../build-muview-Desktop_Qt_5_2_1_clang_64bit-Release"
buildDir='./'
backgroundPictureName="deploy/background-dmg.png"
applicationName="Muview.app"
finalDMGName="Muview2.1.2-MacOSX.dmg"
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
make -j10

echo "Creating Bundle at $PWD/Muview.app"
echo "using qmake from $(which qmake)"
echo "using macdeployqt from $(which macdeployqt)"

macdeployqt $buildDir/$applicationName -verbose=3

# Create a Nice Looking DMG
# http://stackoverflow.com/questions/96882/how-do-i-create-a-nice-looking-dmg-for-mac-os-x-using-command-line-tools

if [ -e pack.temp.dmg ]
	then
	rm pack.temp.dmg
fi
if [ -e $finalDMGName ]
	then
	rm $finalDMGName
fi

hdiutil create pack.temp.dmg -srcfolder $buildDir/$applicationName -format UDRW -volname $title

device=$(hdiutil attach -readwrite -noverify -noautoopen "pack.temp.dmg" | \
         egrep '^/dev/' | sed 1q | awk '{print $1}')

sleep 1

mkdir "/Volumes/${title}/.background"
cp deploy/background-dmg.png "/Volumes/${title}/.background"

echo '
   tell application "Finder"
     tell disk "'${title}'"
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

chmod -Rf go-w /Volumes/"${title}"
sync
sync
hdiutil detach ${device}
hdiutil convert "pack.temp.dmg" -format UDZO -imagekey zlib-level=9 -o "${finalDMGName}"
rm -f pack.temp.dmg 

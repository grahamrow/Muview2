
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
           set position of item "Muview.app" of container window to {212, 282}
           update without registering applications
           delay 5
     end tell
   end tell


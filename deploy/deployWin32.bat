set PATH=C:\Qt\5.7\msvc2015_64\bin;%PATH%
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

qmake.exe -r -config release
nmake.exe
mkdir Muview
move source\muview.exe Muview\Muview.exe
windeployqt.exe Muview\Muview.exe
7z a Muview.7z Muview
rmdir Muview

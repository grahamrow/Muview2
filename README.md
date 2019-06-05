### Muview2
[![Build Status](https://travis-ci.org/grahamrow/Muview2.svg?branch=master)](https://travis-ci.org/grahamrow/Muview2) [![Build status](https://ci.appveyor.com/api/projects/status/ykl226n7alqy0kvf?svg=true)](https://ci.appveyor.com/project/grahamrow/muview2)

MuView2 is an OVF (version 1 and 2) compatible viewer intended to function with Mumax and OOMMF. The viewer provides a fast QT/OpenGL interface that enables users to scrub through loaded files with an animation timeline, eliminating the need for manually generating image sequences and rendering animations using several third party software packages.

### New Features
- Moved to instanced drawing scheme for drastically increased performance.
- Offload glyph orientation/coloring computation on GLSL shaders for huge performance gains.
- Updated to build with Qt 5.12.3.

### Current Features
- Interactive 3D view of vector and scalar inputs (of arbitrary type)
- Slicing for viewing cross-sections of the geometry
- Easy scrubbing (with automatic caching) through loaded files
- Watch folders such that Muview displays new files when they are created
- Save image sequences directly from the loaded set of files

Files may be loaded from command line using standard globbing
```
muview SpinTorqueOscillator.out/m*.ovf
muview OOMMF-Data/Oxs*.omf
```
Otherwise, groups of files or entire directories can be loaded from the GUI.

### Installation
All builds are available for Linux, macOS, and Window at the [releases](https://github.com/grahamrow/Muview2/releases) page.

**Linux**
An [AppImage](https://appimage.org/) bundle is now available for Linux, which is recommended for all Linux users given it's portability and distribution agnosticism. This may be executed directly, possibly requiring the execution flag is set with`chmod +x Muview2-x86_64.AppImage`. Otherwise a self-contained `Muview.tar.gz` bundle is available with all of the necessary Qt libraries included.

**Windows**
The self-contained Windows distribution `Muview.7z` required 7zip for expansion. The required MSVC redistributable is bundled in the same archive as `vcredist_x64.exe`.

**macOS**
Just mount `Muview.dmg` image and drag Muview to the applications folder. Qt libraries are embedded in the .app bundle.

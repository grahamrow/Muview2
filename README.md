### Muview2
[![Build Status](https://travis-ci.org/grahamrow/Muview2.svg?branch=master)](https://travis-ci.org/grahamrow/Muview2) [![Build status](https://ci.appveyor.com/api/projects/status/0av837y4esu4t2gg?svg=true)](https://ci.appveyor.com/project/grahamrow/muview2)

MuView2 is an OVF (version 1 and 2) compatible viewer intended to function with Mumax and OOMMF. The viewer provides a fast QT/OpenGL interface that enables users to scrub through loaded files with an animation timeline, eliminating the need for manually generating image sequences and rendering animations using several third party software packages.

### Current Features
- Interactive 3D view of vector and scalar inputs (of arbitrary type)
- Slicing for viewing cross-sections of the geometry
- Easy scrubbing (with automatic caching) through loaded files
- Watch folders such that Muview displays new files when they are created
- Save image sequences and movies directly from the loaded set of files

Files may be loaded from command line using standard globbing
```
muview SpinTorqueOscillator.out/m*.ovf
muview OOMMF-Data/Oxs*.omf
```
Otherwise, groups of files or entire directories can be loaded from the GUI.

### Installation and Compilation
Until the dust settles, please see release-specific instruction on the [releases](https://github.com/grahamrow/Muview2/releases) page.

### Planned Features
Please let me know what would be useful! I don't plan to add any analysis capabilities.

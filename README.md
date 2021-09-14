VPaint is an **experimental prototype** based on the Vector
Graphics Complex (**VGC**), a technology developed by a
collaboration of researchers at Inria and the University of
British Columbia. It allows you to create resolution-independent
illustrations and animations using innovative techniques.

VPaint is licensed under the **[Apache 2.0 License](https://github.com/dalboris/vpaint/blob/master/LICENSE)**.

More Info:
- **[www.vpaint.org](https://www.vpaint.org)**
- **[www.vgc.io](https://www.vgc.io)**

<img src="https://www.vgc.io/images/shared-edges-intro-01-24fps.gif" width="640px">
<img src="https://www.vgc.io/images/shared-edges-others-01-24fps.gif" width="640px">
<img src="https://www.vgc.io/images/glue-01-3-24fps.gif" width="640px">
<img src="https://www.vgc.io/images/glue-02-24fps.gif" width="640px">
<img src="https://www.vgc.io/images/sculpt-01-24fps.gif" width="640px">
<img src="https://www.vgc.io/images/depth-ordering-02-24fps.gif" width="640px">
<img src="https://www.vgc.io/images/depth-ordering-01-24fps.gif" width="640px">
<img src="https://www.vgc.io/images/star-split-360p-24fps.gif" width="640px">
<img src="https://www.vgc.io/images/star-split-3D-360p-24fps.gif" width="640px">


## Disclaimer

VPaint is considered BETA, and always will: it
lacks plenty of useful features commonly found in other editors, and you
should expect glitches and crashes once in a while. It is distributed
primarily for research purposes, and for curious artists interested in early
testing of cutting-edge but unstable technology.

## Future Releases

Because VPaint is just a prototype, we may or may not release new versions.
However, we are currently developing two new apps, called VGC Illustration and
VGC Animation, based on what we learned from VPaint. These new apps will significantly
improve performance compared to VPaint, and have more features.

**If you would like to be notified if/when
a new version of VPaint is released, or when VGC Illustration and VGC Animation are released**, you
can subscribe to the newsletter at **[vgc.io](https://www.vgc.io/news)**. Thanks for your interest, and have fun
testing VPaint!

## Build Instructions

Prerequisites:
- **C++11**: We recommend Visual Studio 2017 on Windows, and any recent version of Clang/GCC on macOS/Linux.
- **Qt 5.12**: We recommend installing the latest Qt 5.12.x version using the [Qt installer](https://www.qt.io/download-qt-installer).
- **GLU**: This is already installed on Windows, macOS, and many Linux distributions. However, on Ubuntu, you need to install it yourself via `sudo apt-get install libglu1-mesa-dev`.

### Using qmake (application only)

#### Windows 7/8/10, Visual Studio 2017 64bit, Qt 5.12.6

```
git clone https://github.com/dalboris/vpaint.git
mkdir build && cd build
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat" -arch=amd64
C:\Qt\5.12.6\msvc2017_64\bin\qmake.exe ..\vpaint\src
nmake
set PATH=%PATH%;C:\Qt\5.12.6\msvc2017_64\bin
Gui\release\VPaint.exe
```

#### macOS 10.14.6, Xcode 10.3, Qt 5.12.6

```
git clone https://github.com/dalboris/vpaint.git
mkdir build && cd build
~/Qt/5.12.6/clang_64/bin/qmake ../vpaint/src
make
./Gui/VPaint.app/Contents/MacOS/VPaint
```

#### Ubuntu 18.04, GCC, Qt 5.12.6

```
sudo apt-get install build-essential libglu1-mesa-dev
git clone https://github.com/dalboris/vpaint.git
mkdir build && cd build
~/Qt/5.12.6/gcc_64/bin/qmake ../vpaint/src
make
./Gui/VPaint
```

### Using CMake (application and library)

```
git clone https://github.com/dalboris/vpaint.git
cd vpaint
mkdir build && cd build
cmake ..
cmake --build . --config Release
./src/Gui/Release/VPaint
```

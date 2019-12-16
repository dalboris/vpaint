<img src="https://github.com/dalboris/vpaint/blob/master/src/Gui/images/aboutlogo.png" width="660px">

VPaint is an **experimental prototype** based on the Vector
Graphics Complex (**VGC**), a technology developed by a
collaboration of researchers at Inria and the University of
British Columbia. It allows you to create resolution-independent
illustrations and animations using innovative techniques.

VPaint is licensed under the **[Apache 2.0 License](https://github.com/dalboris/vpaint/blob/master/LICENSE)**.

More Info:
- **[www.vpaint.org](https://www.vpaint.org)**
- **[www.vgc.io](https://www.vgc.io)**

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
can sign up for a VGC account at **[vgc.io](https://www.vgc.io)**. Thanks for your interest, and have fun
testing VPaint!

## Build Instructions

Dependencies:
- **C++11**: We recommend Visual Studio 2017 on Windows, and any recent version of Clang/GCC on macOS/Linux.
- **Qt 5.12**: We recommend installing the latest Qt 5.12.x version using the [Qt installer](https://www.qt.io/download-qt-installer).
- **GLU**: This is already installed on Windows, macOS, and many Linux distributions. However, on Ubuntu, you need to install it yourself via `sudo apt-get install libglu1-mesa-dev`.

#### Ubuntu 18.04, GCC, Qt 5.12.6

```
sudo apt-get install build-essential libglu1-mesa-dev
git clone https://github.com/dalboris/vpaint.git
cd vpaint/src
mkdir build && cd build
~/Qt/5.12.6/gcc_64/bin/qmake ..
make
./Gui/VPaint
```

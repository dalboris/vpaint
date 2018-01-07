![VPaint](https://github.com/dalboris/vpaint/blob/master/src/Gui/images/logobeta.png)

[![Join the chat at https://gitter.im/dalboris/vpaint](https://badges.gitter.im/dalboris/vpaint.svg)](https://gitter.im/dalboris/vpaint?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

# VPAINT BECOMES VGC ANIMATION

The future of VPaint is VGC Illustration and VGC Animation. See [www.vgc.io](https://www.vgc.io/) for more info. Github:  [https://github.com/vgc-io/vgc](https://github.com/vgc-io/vgc).

This repository is obsolete and kept for archival purposes only. 


# ABOUT

VPaint was a vector graphics and 2D animation app based on the
[Vector Animation Complex](http://www.borisdalstein.com/research/vac/) 
technology. Visit [vpaint.org](http://vpaint.org) to learn more,
or visit the [VGC forums](http://forums.vgc.io/). VPaint is proud to 
be open source software licensed under 
[the MIT license](https://github.com/dalboris/vpaint/blob/master/LICENSE.MIT).


# KNOWN ISSUES

- On Mac OS X 10.10 Yosemite and above, maximizing the window is poorly supported. If you want to exit maximizing, hide the toolbar and action bar with "View > Toolbar" and "View > Action Bar" so you can access the minimize button again.

- On Mac OS X, if you have a retina display, then the pointer drawn by VPaint is not at the same location as your mouse cursor. It doesn't prevent using VPaint but it is quite inconvenient. See [#22](https://github.com/dalboris/vpaint/issues/22) for details.


# HOW TO CONTRIBUTE

One of the most helpful way to contribute is to spread the word about VPaint! Share it on Facebook, tweet about it, or write a blog post about it. Don't forget to join the [mailing list](http://www.vpaint.org/#about) so you don't miss out when new features are available. The more people are excited about VPaint, the more its developers work on it, the greater it becomes.

If you are a developer and want to contribute code, this is awesome. We can't wait to see the improvements you will bring to VPaint. Below are a few guidelines for effective contributions:

1. Only contributions licensed under the MIT will be accepted. To do this, simply insert your copyright notice in [the COPYRIGHT file](https://github.com/dalboris/vpaint/blob/master/COPYRIGHT). The copyright notice must
include your full name.

2. If you want to fix a bug, feel free to go ahead without prior approval: fork the repository, create a new branch fixing the bug, then submit a pull request. We might suggest a few modifications or ask clarifications before merging, but be confident that it will be merged if it does fix a bug. Don't hesitate to ask for guidance and/or assistance at any time, either on the bug report or in the [dev chatroom](https://gitter.im/dalboris/vpaint).

3. If you want to implement a new feature, please discuss it first (open a new feature request or reply to an existing one). This makes sure we are all on the same page, prevents software bloat, and allows us to provide guidance ahead of time.

4. Please do not contribute code refactoring, it will be ignored. Yes, the code of VPaint is a bit of a mess and requires refactoring, but this is a task that will be done by your dedicated project lead, when the time is right.


# HOW TO COMPILE

## Linux

(tested on Kubuntu 15.04 64bit)

1. Install GCC 4.8 or higher. On Ubuntu, you can do this with:
     sudo apt-get install build-essential
2. Install GLU for development. On Ubuntu, you can do this with:
     sudo apt-get install libglu1-mesa-dev
3. Download and install Qt 5.4 Community from http://www.qt.io/
4. Open VPaint.pro with Qt Creator
5. Specify GCC 4.8+ as your compiler
6. Build > Run


## Mac OS X

(tested on Mac OS X Yosemite 10.10.4, running on a MacBook Pro 15-inch Early 2008)

- Note 1: VPaint will -NOT- compile with clang or GCC 4.2 (provided by default on Mac OS X).

- Note 2: There are bugs in Qt 5.1 to 5.4 with Yosemite, hence Qt 5.5+ is recommended.

1. Install XCode
2. Install GCC 4.8 via homebrew, following these instructions:
     http://www-scf.usc.edu/~csci104/20142/installation/gccmac.html
3. Download and install Qt 5.5 Community from http://www.qt.io/
4. Create a new Qt mkspec for GCC 4.8 following these instructions:
    a. Go to the folder /Users/<username>/Qt/5.5/clang_64/mkspecs
    b. Duplicate the folder macx-g++42 and rename the copy macx-g++48
    c. In the folder macx-g++48, edit the file qmake.conf by setting these two values:
        QMAKE_CC  = gcc-4.8
        QMAKE_CXX = g++-4.8
5. Open Qt Creator
6. Go to Qt Creator > Preferences > Build and Run > Compiler > Add > GCC
    Name: G++ 4.8
    Compiler path: /usr/local/bin/g++-4.8
7. Go to Qt Creator > Preferences > Build and Run > Kit > Add
    Name: Desktop Qt 5.5.0 G++ 4.8
    Compiler: G++ 4.8
    Qt mkspec: macx-g++48
8. Open VPaint.pro
9. Build > Run


## Windows

(tested on Windows 7 64bit)

- Note 1: In order to support more users, it is recommended to compile as 32bit code, as
          the resultant executables can run on both 32bit and 64bit versions of Windows.

- Note 2: Windows XP and lower are not supported.

- Note 3: VPaint will -NOT- compile with MinGW 4.8 (either 32bit and 64bit)
        or Visual Studio 2010 or lower.

1. Make sure that Visual Studio 2012 or higher is installed on your system
2. Download and install Qt 5.4 Community from http://www.qt.io/.  Make
   sure to use the OpenGL version (not ANGLE), and that it corresponds to
   the version of Visual Studio you have.
3. Open VPaint.pro with Qt Creator
4. Specify MSVC as your compiler
5. Build > Run

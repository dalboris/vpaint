#  ABOUT VPAINT

- Contact: Boris Dalstein <dalboris@gmail.com>
- Website: http://www.vpaint.org

VPaint is an experimental vector graphics editor based on the
Vector Animation Complex technology. It allows you to create
resolution-independent illustrations and animations using 
innovative techniques.

# KNOWN ISSUES

## General

- VPaint 1.5 is considered a BETA version. It may crash unexpectedly, so it is recommended to save often. The most unstable features or still in development are marked with the text [Beta] in the menus.

## MacOS X

- On MacOS 10.10 Yosemite, maximizing the window is poorly supported. If you want to exit maximizing, hide the toolbar and action bar with "View > Toolbar" and "View > Action Bar" so you can access the minimize button again.

- Some users have experienced a mouse pointer issue: the pointer drawn by VPaint is not at the same location as your mouse cursor. It doesn't prevent using VPaint but it is quite inconvenient. 

- In the status bar (at the bottom), whenever "CTRL" is mentioned, you should use the COMMAND key instead. 

- To delete something, you have to use the "Delete" key (i.e., "delete forward"). If you only have the "Backspace" key on your keyboard, you have to use Fn+Backspace instead.


#  LICENSE

VPaint is primarily licensed under the MIT License.
See COPYING for details.


# HOW TO COMPILE

## Linux

(tested on Kubuntu 15.04 64bit)

1. Install GCC 4.8 or higher. On Ubuntu, you can do this with:
     sudo apt-get install build-essential
2. Install GLU for development. On Ubuntu, you can do this with:
     sudo apt-get install libglu1-mesa-dev
3. Download and install Qt 5.4 Community from http://www.qt.io/
4. Open src/VPaint.pro with Qt Creator
5. Specify GCC 4.8+ as your compiler
6. Build > Run


## MacOS X

(tested on MacOS X Yosemite 10.10.4, on MacBook Pro 15-inch Early 2008)

- Note 1: VPaint will -NOT- compile with clang or GCC 4.2 (provided by default on MacOS X).

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
8. Open src/VPaint.pro
9. Build > Run


## Windows

(tested on Windows 7 64bit)

- Note 1: To support more user, compiling in 32bit is recommended, as
          it can run on both 32bit and 64bit versions of Windows.

- Note 2: Windows XP and lower are not supported.

- Note 3: VPaint will -NOT- compile with MinGW 4.8 (either 32bit and 64bit)
        or Visual Studio 2010 or lower.

1. Make sure that Visual Studio 2012 or higher is installed on your system
2. Download and install Qt 5.4 Community from http://www.qt.io/.  Make
   sure to use the OpenGL version (not ANGLE), and that it corresponds to
   the version of Visual Studio you have.
3. Open src/VPaint.pro with Qt Creator
4. Specify MSVC as your compiler
5. Build > Run

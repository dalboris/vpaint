#!/bin/bash
#
# This script creates an AppImage from the given build.
#
# Usage:
#
# sudo apt-get install build-essential libglu1-mesa-dev
# git clone https://github.com/dalboris/vpaint.git
# mkdir build && cd build
# ~/Qt/5.12.6/gcc_64/bin/qmake ../vpaint/src
# make
# chmod u+x ../vpaint/tools/linux/deploy.sh
# ../vpaint/tools/linux/deploy.sh
#

# Parse Makefile to get QMAKE location
QMAKE=$(grep "QMAKE *=" Makefile | sed "s/QMAKE *= *//g")

# Get app version (linuxdeployqt uses this for naming the file)
VPAINT_VERSION=$(grep "VERSION = " ../vpaint/src/Gui/Gui.pro | sed "s/VERSION *= *//g")
export VERSION=${VPAINT_VERSION}

# Create appdir structure
rm -rf appdir
mkdir appdir
mkdir appdir/usr
mkdir appdir/usr/bin
mkdir appdir/usr/lib
mkdir appdir/usr/share
mkdir appdir/usr/share/applications
mkdir appdir/usr/share/icons
mkdir appdir/usr/share/icons/hicolor

# Copy app binary
cp Gui/VPaint appdir/usr/bin

# Create desktop file
echo "[Desktop Entry]
Name=VPaint
Comment=Vector-Based Animation Editor
Exec=VPaint
Icon=VPaint
Terminal=false
Type=Application
Categories=Graphics;" > appdir/usr/share/applications/VPaint.desktop

# Copy icons
for size in 16 32 48 256; do
    mkdir appdir/usr/share/icons/hicolor/${size}x${size}
    cp ../vpaint/src/Gui/images/icon-${size}.png appdir/usr/share/icons/hicolor/${size}x${size}/VPaint.png
done

# We need to manually download and compile OpenSLL 1.1.1, since Qt 5.12.4+
# requires OpenSSL 1.1.1, but Ubuntu 16.04 only ships OpenSSL 1.0.2.
#
# See:
# https://www.qt.io/blog/2019/06/17/qt-5-12-4-released-support-openssl-1-1-1
# https://askubuntu.com/questions/1102803/how-to-upgrade-openssl-1-1-0-to-1-1-1-in-ubuntu-18-04
#
OPENSSL_VERSION=openssl-1.1.1d
if [ ! -d openssl ]; then
    DIR=$(pwd)
    mkdir openssl && cd openssl
    wget https://www.openssl.org/source/${OPENSSL_VERSION}.tar.gz
    tar -zxf ${OPENSSL_VERSION}.tar.gz
    cd ${OPENSSL_VERSION}
    ./config
    make
    cd ${DIR}
fi
for libname in crypto ssl; do
    cp openssl/${OPENSSL_VERSION}/lib${libname}.so.1.1 appdir/usr/lib
    ln -s lib${libname}.so.1.1 appdir/usr/lib/lib${libname}.so
done

# Execute linuxdeployqt
wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt-continuous-x86_64.AppImage
./linuxdeployqt-continuous-x86_64.AppImage appdir/usr/share/applications/VPaint.desktop -qmake=${QMAKE} -appimage

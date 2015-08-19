#-------------------------------------------------
#
# Project created by QtCreator 2015-08-11T22:59:34
#
#-------------------------------------------------

QT       -= core gui

TARGET = GLEW
TEMPLATE = lib
CONFIG += staticlib

SOURCES += glew.c

HEADERS += glew.h \
           glxew.h \
           wglew.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

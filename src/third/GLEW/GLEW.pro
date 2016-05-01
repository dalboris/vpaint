TEMPLATE = lib
CONFIG -= qt

SOURCES += glew.c

HEADERS += glew.h \
           glxew.h \
           wglew.h

include($$OUT_PWD/.config.pri)

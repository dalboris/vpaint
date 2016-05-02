TEMPLATE = app

LIB_DEPENDS = \
    VPaint

win32 {
    # Set icon
    RC_ICONS += vpaint.ico

    # Embed manifest file
    win32: CONFIG += embed_manifest_exe
}
else:macx {
    # Set icon
    ICON = vpaint.icns

    # Use a custom Info.plist
    QMAKE_INFO_PLIST = Info.plist

    # Add file icons into the application bundle resources
    FILE_ICONS.files = vec.icns
    FILE_ICONS.path = Contents/Resources
    QMAKE_BUNDLE_DATA += FILE_ICONS
}

SOURCES += main.cpp

include($$OUT_PWD/.config.pri)

# Link against GLU for Linux (XXX to be removed)
unix:!macx: LIBS += -lGLU

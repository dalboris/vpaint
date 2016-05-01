# Subdir template. No need to put them in any particular order: appropriate
# dependencies are automatically generated.
TEMPLATE = subdirs
SUBDIRS = \
    app \
    libs \
    third

# Include automatically generated config file
include($$OUT_PWD/.config.pri)

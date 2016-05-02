# Configure
include(configure/configure.pri)

# Subdir template
TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = src tests

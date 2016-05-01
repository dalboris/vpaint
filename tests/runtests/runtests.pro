TEMPLATE = app
CONFIG += c++11
QT += widgets

DEFINES += QMAKE_PWD=\\\"$$PWD\\\"
DEFINES += QMAKE_OUT_PWD=\\\"$$OUT_PWD\\\"
DEFINES += QMAKE_QMAKE_QMAKE=\\\"$$QMAKE_QMAKE\\\"
DEFINES += QMAKE_QMAKESPEC=\\\"$$QMAKESPEC\\\"

DEFINES += QMAKE_CONFIG=\\\"$$(CONFIG)\\\"

RESOURCES += resources.qrc

SOURCES += \
    main.cpp \
    MainWindow.cpp \
    TestRunner.cpp \
    DirUtils.cpp \
    DirTestItem.cpp \
    FileTestItem.cpp \
    TestItem.cpp \
    TestTreeModel.cpp \
    TestTreeView.cpp \
    OutputWidget.cpp \
    RunButton.cpp \
    TestTreeSelectionModel.cpp \
    DependsUtils.cpp \
    ConfigUtils.cpp \
    CommandLineApplication.cpp \
    GuiApplication.cpp

HEADERS += \
    MainWindow.h \
    TestRunner.h \
    DirUtils.h \
    DirTestItem.h \
    FileTestItem.h \
    TestItem.h \
    TestTreeModel.h \
    TestTreeView.h \
    OutputWidget.h \
    RunButton.h \
    TestTreeSelectionModel.h \
    DependsUtils.h \
    ConfigUtils.h \
    CommandLineApplication.h \
    GuiApplication.h


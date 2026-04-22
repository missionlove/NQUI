QT += core gui widgets charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
DEFINES += QT_DEPRECATED_WARNINGS

win32: QMAKE_CXXFLAGS += /utf-8

include(../Demos/common.pri)

TARGET = NApp

INCLUDEPATH += $$PWD/../NPlugins/NDashboard
INCLUDEPATH += $$PWD/../NPlugins/NFramelessWidget

win32 {
    NFRAMELESS_LIB_ARCH = $$QMAKE_TARGET_ARCH
    equals(NFRAMELESS_LIB_ARCH, x86_64) {
        NFRAMELESS_LIB_ARCH = x64
    }

    CONFIG(release, debug|release) {
        NFRAMELESS_LIB_DIR = $$PWD/../build/libs/$${NFRAMELESS_LIB_ARCH}/release
        NFRAMELESS_LIB_NAME = NFramelessWidget
        ND_LIB_DIR = $$PWD/../build/libs/$${NFRAMELESS_LIB_ARCH}/release
        ND_LIB_NAME = NDashboard
    } else {
        NFRAMELESS_LIB_DIR = $$PWD/../build/libs/$${NFRAMELESS_LIB_ARCH}/debug
        NFRAMELESS_LIB_NAME = NFramelessWidgetd
        ND_LIB_DIR = $$PWD/../build/libs/$${NFRAMELESS_LIB_ARCH}/debug
        ND_LIB_NAME = NDashboardd
    }

    LIBS += -L$${NFRAMELESS_LIB_DIR} -l$${NFRAMELESS_LIB_NAME}
    LIBS += -L$${ND_LIB_DIR} -l$${ND_LIB_NAME}
    QMAKE_POST_LINK += copy /Y \"$${NFRAMELESS_LIB_DIR}\\$${NFRAMELESS_LIB_NAME}.dll\" \"$${DESTDIR}\\$${NFRAMELESS_LIB_NAME}.dll\"
    QMAKE_POST_LINK += && copy /Y \"$${ND_LIB_DIR}\\$${ND_LIB_NAME}.dll\" \"$${DESTDIR}\\$${ND_LIB_NAME}.dll\"
}

SOURCES += \
    main.cpp \
    nappwindow.cpp

HEADERS += \
    nappwindow.h

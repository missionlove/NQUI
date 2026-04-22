QT += widgets charts

TEMPLATE = lib
DEFINES += NDASHBOARD_LIBRARY
DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

win32: QMAKE_CXXFLAGS += /utf-8

SOURCES += \
    nappshell.cpp \
    ndashboardwidgetbase.cpp \
    ndashboardpushbuttonbase.cpp \
    ndashboardcontentlayout.cpp \
    ndashboardheader.cpp \
    npagetitlewithfilter.cpp \
    nprimaryactionbutton.cpp \
    nnotificationbellbutton.cpp \
    nsidebarnavigation.cpp \
    nsidebarnavitem.cpp \
    nuserprofilecompact.cpp \
    ntrendindicator.cpp \
    nmetricsummarycard.cpp \
    ntimeserieslinechartpanel.cpp \
    ncirculargaugecard.cpp \
    nsparklinemetriccard.cpp \
    ngroupedbarchartpanel.cpp

HEADERS += \
    NDashboard_global.h \
    nappshell.h \
    ndashboardwidgetbase.h \
    ndashboardpushbuttonbase.h \
    ndashboardcontentlayout.h \
    ndashboardheader.h \
    npagetitlewithfilter.h \
    nprimaryactionbutton.h \
    nnotificationbellbutton.h \
    nsidebarnavigation.h \
    nsidebarnavitem.h \
    nuserprofilecompact.h \
    ntrendindicator.h \
    nmetricsummarycard.h \
    ntimeserieslinechartpanel.h \
    ncirculargaugecard.h \
    nsparklinemetriccard.h \
    ngroupedbarchartpanel.h

CONFIG += no_debug_release
DESTDIR = $$PWD/../../build/libs/
DIST_DIR = $$PWD/../../dist
OBJECTS_DIR = $$PWD/../../build/temp/$${TARGET}/obj
MOC_DIR = $$PWD/../../build/temp/$${TARGET}/moc
RCC_DIR = $$PWD/../../build/temp/$${TARGET}/rcc
UI_DIR = $$PWD/../../build/temp/$${TARGET}/ui

win32 {
    QMAKE_TARGET_ARCH = x86
} else {
    QMAKE_TARGET_ARCH = x86_64
}

CONFIG(release, debug|release) {
    DESTDIR = $${DESTDIR}/$${QMAKE_TARGET_ARCH}/release
} else {
    DESTDIR = $${DESTDIR}/$${QMAKE_TARGET_ARCH}/debug
    TARGET = $${TARGET}d
}

win32: PWD = $$replace(PWD, "/", "\\")
win32: DESTDIR = $$replace(DESTDIR, "/", "\\")
win32: DIST_DIR = $$replace(DIST_DIR, "/", "\\")

CONFIG(release, debug|release) {
    win32:QMAKE_POST_LINK += if not exist \"$${DIST_DIR}\\lib\" mkdir \"$${DIST_DIR}\\lib\" && if not exist \"$${DIST_DIR}\\include\" mkdir \"$${DIST_DIR}\\include\" && copy /Y \"$${DESTDIR}\\$${TARGET}.dll\" \"$${DIST_DIR}\\lib\\$${TARGET}.dll\" && copy /Y \"$${DESTDIR}\\$${TARGET}.lib\" \"$${DIST_DIR}\\lib\\$${TARGET}.lib\" && copy /Y \"$${PWD}\\*.h\" \"$${DIST_DIR}\\include\\\"
}

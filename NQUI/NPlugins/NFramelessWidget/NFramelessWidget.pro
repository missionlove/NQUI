QT += widgets

TEMPLATE = lib
DEFINES += NFRAMELESSWIDGET_LIBRARY

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    nframelesswidget.cpp

HEADERS += \
    NFramelessWidget_global.h \
    nframelesswidget.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target




CONFIG     += no_debug_release                         # 不会生成debug 和 release 文件目录
DESTDIR     = $$PWD/../../build/libs/                 # 指定编译最终文件的生成路径
DIST_DIR    = $$PWD/../../dist                            # 库的发布路径
OBJECTS_DIR = $$PWD/../../build/temp/$${TARGET}/obj       # obj中间文件存放路径
MOC_DIR     = $$PWD/../../build/temp/$${TARGET}/moc       # moc中间文件路径
RCC_DIR     = $$PWD/../../build/temp/$${TARGET}/rcc       # rcc资源文件路径
UI_DIR      = $$PWD/../../build/temp/$${TARGET}/ui        # ui_XXX.h 中间文件存放路径


# 在release模式下生成debug信息：.pdb
#QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
#QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO

# 区分win32和x64
win32 {
    QMAKE_TARGET_ARCH = x86
} else {
    QMAKE_TARGET_ARCH = x64
}


# 区分release和debug版本，debug文件后缀自动添加d
CONFIG(release, debug|release) {
    # release版本
    DESTDIR = $${DESTDIR}/$${QMAKE_TARGET_ARCH}/release

    TARGET = $${TARGET}
} else {
    # debug版本
    DESTDIR = $${DESTDIR}/$${QMAKE_TARGET_ARCH}/debug
    TARGET = $${TARGET}d
}



win32: PWD= $$replace(PWD,"/","\\")
win32: DESTDIR =  $$replace(DESTDIR,"/","\\")
win32: DIST_DIR = $$replace(DIST_DIR,"/","\\")

CONFIG(release, debug|release) {
    win32:QMAKE_POST_LINK += if not exist \"$${DIST_DIR}\\lib\" mkdir \"$${DIST_DIR}\\lib\" && if not exist \"$${DIST_DIR}\\include\" mkdir \"$${DIST_DIR}\\include\" && copy /Y \"$${DESTDIR}\\$${TARGET}.dll\" \"$${DIST_DIR}\\lib\\$${TARGET}.dll\" && copy /Y \"$${DESTDIR}\\$${TARGET}.lib\" \"$${DIST_DIR}\\lib\\$${TARGET}.lib\" && copy /Y \"$${PWD}\\*.h\" \"$${DIST_DIR}\\include\\\"
}


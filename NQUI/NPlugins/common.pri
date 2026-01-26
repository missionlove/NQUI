
# 在release模式下生成debug信息：.pdb
QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO

# 区分win32和x64
win32 {
    QMAKE_TARGET_ARCH = x86
} else {
    QMAKE_TARGET_ARCH = x86_64
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


CONFIG     += no_debug_release                   # 不会生成debug 和 release 文件目录
DESTDIR     = $$PWD/../build/bin                 # 指定编译最终文件的生成路径
OBJECTS_DIR = $$PWD/../build/$${TARGET}/obj       # obj中间文件存放路径
MOC_DIR     = $$PWD/../build/$${TARGET}/moc       # moc中间文件路径
RCC_DIR     = $$PWD/../build/$${TARGET}/rcc       # rcc资源文件路径
UI_DIR      = $$PWD/../build/$${TARGET}/ui        # ui_XXX.h 中间文件存放路径



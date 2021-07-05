DEFINES += DQAPPLICATION_CLASS='QApplication'

HEADERS += \
    $$PWD/SingleApplication                     \
    $$PWD/singleapplication.h                   \
    $$PWD/singleapplication_p.h                 \

SOURCES += \
    $$PWD/singleapplication.cpp                 \
    $$PWD/singleapplication_p.cpp               \

INCLUDEPATH += $$PWD

win32 {
    msvc:LIBS += Advapi32.lib
    gcc:LIBS += -ladvapi32
}

DISTFILES += \
    $$PWD/README.md                             \
    $$PWD/Windows.md                            \
    $$PWD/CHANGELOG.md                          \

SINGLE_LIB.files += \
    $$PWD/SingleApplication                     \
    $$PWD/singleapplication.h                   \

SINGLE_LIB.path = $$INSTALL_HEADER_DIR


SINGLE_LIB_PRIV.files += \
    $$PWD/singleapplication_p.h                 \

SINGLE_LIB_PRIV.path = $$INSTALL_HEADER_DIR/private/




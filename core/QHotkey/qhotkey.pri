CONFIG += C++11

QHOTKEY_PUBLIC_HEADERS += \
    $$PWD/qhotkey.h                     \
    $$PWD/QHotkey

HEADERS += \
    $$QHOTKEY_PUBLIC_HEADERS            \
    $$PWD/qhotkey_p.h

SOURCES += \
    $$PWD/qhotkey.cpp

mac: SOURCES += $$PWD/qhotkey_mac.cpp
else:win32: SOURCES += $$PWD/qhotkey_win.cpp
else:unix: SOURCES += $$PWD/qhotkey_x11.cpp


QHOTKEY.files = \
    $$QHOTKEY_PUBLIC_HEADERS            \


TEMPLATE = lib
TARGET = graceful


QT += core network widgets dbus
CONFIG += no_keywords c++11


INSTALL_HEADER_DIR = /usr/include/graceful/


include($$PWD/dbus/dbus.pri)
include($$PWD/power/power.pri)
include($$PWD/SingleApplication/singleapplication.pri)


target.path = /lib/


HEADERS += \
    $$PWD/globals.h                             \
    $$PWD/settings.h                            \
    $$PWD/translator.h                          \
    $$PWD/screensaver.h \
    notification.h \
    notification_p.h


SOURCES += \
    $$PWD/settings.cpp                          \
    $$PWD/translator.cpp                        \
    $$PWD/screensaver.cpp \
    notification.cpp


INSTALL_HEADERS.files = \
    $$PWD/globals.h                             \

INSTALL_HEADERS.path = $$INSTALL_HEADER_DIR


INSTALLS += \
    target                                      \
    INSTALL_HEADERS                             \
    SINGLE_LIB SINGLE_LIB_PRIV                  \


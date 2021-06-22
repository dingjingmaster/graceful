TEMPLATE    = lib
TARGET      = graceful


QT          += core network widgets xml dbus x11extras KWindowSystem
CONFIG      += no_keywords c++11 link_pkgconfig
PKGCONFIG   += Qt5Xdg
DEFINES     += COMPILE_LIBGRACEFUL

INCLUDEPATH += $$PWD/


INSTALL_HEADER_DIR = /usr/include/graceful/


include($$PWD/dbus/dbus.pri)
include($$PWD/power/power.pri)
include($$PWD/SingleApplication/singleapplication.pri)


target.path = /lib/


HEADERS += \
    $$PWD/globals.h                             \
    $$PWD/settings.h                            \
    $$PWD/translator.h                          \
    $$PWD/screensaver.h                         \
    $$PWD/notification.h                        \
    $$PWD/notification_p.h                      \
    $$PWD/application.h


SOURCES += \
    $$PWD/settings.cpp                          \
    $$PWD/translator.cpp                        \
    $$PWD/screensaver.cpp                       \
    $$PWD/notification.cpp \
    $$PWD/application.cpp


INSTALL_HEADERS.files = \
    $$PWD/globals.h                             \
    $$PWD/settings.h                            \
    $$PWD/translator.h                          \
    $$PWD/application.h                         \
    $$PWD/screensaver.h                         \
    $$PWD/notification.h                        \


INSTALL_PRIVATE_HEADERS.files = \
    $$PWD/notification_p.h                      \


INSTALL_HEADERS.path = $$INSTALL_HEADER_DIR
INSTALL_PRIVATE_HEADERS.path = $$INSTALL_HEADER_DIR/private/


INSTALLS += \
    target                                      \
    POWER_LIB                                   \
    INSTALL_HEADERS                             \
    INSTALL_PRIVATE_HEADERS                     \
    SINGLE_LIB SINGLE_LIB_PRIV                  \


TEMPLATE        = lib
TARGET          = graceful

QT              += core network widgets xml dbus x11extras KWindowSystem widgets-private
PKGCONFIG       += Qt5Xdg gio-2.0 glib-2.0
DEFINES         += COMPILE_LIBGRACEFUL
CONFIG          += no_keywords c++11 link_pkgconfig sharedlib
LIBS            += -lprocps -lXss -lX11
QMAKE_CXXFLAGS  += -Werror=return-type -Werror=return-local-addr -Werror=uninitialized -Werror=unused-label -execution-charset:utf-8

INCLUDEPATH     += $$PWD/


INSTALL_HEADER_DIR = /usr/include/graceful/

include($$PWD/log/log.pri)
include($$PWD/file/file.pri)
include($$PWD/dbus/dbus.pri)
include($$PWD/power/power.pri)
include($$PWD/common/common.pri)
include($$PWD/QHotkey/qhotkey.pri)
include($$PWD/SingleApplication/singleapplication.pri)
include($$PWD/desktopfiles-model/desktopfiles-model.pri)


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
    $$PWD/notification.cpp                      \
    $$PWD/application.cpp


INSTALL_HEADERS.files = \
    $$LOG_HEADERS                               \
    $$QHOTKEY_PUBLIC_HEADERS                    \
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

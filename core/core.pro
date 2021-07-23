TEMPLATE        = lib
TARGET          = graceful

QT              += core network widgets xml dbus x11extras KWindowSystem widgets-private
DEFINES         += COMPILE_LIBGRACEFUL
CONFIG          += no_keywords c++11 link_pkgconfig sharedlib
LIBS            += -lprocps -lXss -lX11
PKGCONFIG       += Qt5Xdg gio-2.0 glib-2.0 gio-unix-2.0
QMAKE_CXXFLAGS  += -Werror=return-type -Werror=return-local-addr -Werror=uninitialized -Werror=unused-label -execution-charset:utf-8

INCLUDEPATH     += $$PWD/


INSTALL_HEADER_DIR = /usr/include/graceful/

include($$PWD/log/log.pri)
include($$PWD/file/file.pri)
include($$PWD/dbus/dbus.pri)
include($$PWD/utils/utils.pri)
include($$PWD/power/power.pri)
include($$PWD/common/common.pri)
include($$PWD/QHotkey/qhotkey.pri)
include($$PWD/icon-view/icon-view.pri)
include($$PWD/file-model/file-model.pri)
include($$PWD/qgsettings/qgsettings.pri)
include($$PWD/SingleApplication/singleapplication.pri)


target.path = /lib/


HEADERS += \
    $$PWD/globals.h                             \
    $$PWD/settings.h                            \
    $$PWD/translator.h                          \
    $$PWD/application.h                         \
    $$PWD/screensaver.h                         \
    $$PWD/notification.h                        \
    $$PWD/notification_p.h                      \
    $$PWD/thumbnail-manager.h                   \
    $$PWD/regular-file-type.h                   \
    $$PWD/global-settings.h


SOURCES += \
    $$PWD/settings.cpp                          \
    $$PWD/translator.cpp                        \
    $$PWD/application.cpp                       \
    $$PWD/screensaver.cpp                       \
    $$PWD/notification.cpp                      \
    $$PWD/thumbnail-manager.cpp                 \
    $$PWD/global-settings.cpp


CORE_HEADERS = \
    $$PWD/globals.h                             \
    $$PWD/settings.h                            \
    $$PWD/translator.h                          \
    $$PWD/application.h                         \
    $$PWD/screensaver.h                         \
    $$PWD/notification.h                        \
    $$PWD/notification_p.h                      \
    $$PWD/global-settings.h                     \
    $$PWD/thumbnail-manager.h                   \


INSTALL_HEADERS.files = \
    $$LOG_HEADERS                               \
    $$CORE_HEADERS                              \
    $$INSTALL_UTILS                             \
    $$ICON_VIEW_HEADER                          \
    $$FILE_FILE_HEADERS                         \
    $$QSETTINGS_HEADERS                         \
    $$FILE_MODEL_HEADERS                        \
    $$QHOTKEY_PUBLIC_HEADERS                    \


INSTALL_PRIVATE_HEADERS.files = \
    $$PWD/notification_p.h                      \


INSTALL_HEADERS.path = $$INSTALL_HEADER_DIR
INSTALL_PRIVATE_HEADERS.path = $$INSTALL_HEADER_DIR/private/


PC_FILE.path = $$[QT_INSTALL_LIBS]/pkgconfig
PC_FILE.files = $$PWD/graceful.pc


INSTALLS += \
    target                                      \
    PC_FILE                                     \
    POWER_LIB                                   \
    INSTALL_HEADERS                             \
    INSTALL_PRIVATE_HEADERS                     \
    SINGLE_LIB SINGLE_LIB_PRIV                  \


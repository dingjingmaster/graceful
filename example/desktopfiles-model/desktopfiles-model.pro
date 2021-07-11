TEMPLATE        = app
TARGET          = desktopfile-model

QT              += core gui widgets

INCLUDEPATH     = $$PWD/../../core/

CONFIG          += no_keywords c++11 link_pkgconfig sharedlib
PKGCONFIG       += Qt5Xdg gio-2.0 glib-2.0
LIBS            += -lprocps -lXss -lX11 -L$$OUT_PWD/../../core/ -lgraceful

SOURCES += \
    main.cpp



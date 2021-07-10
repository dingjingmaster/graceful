TEMPLATE        = app
TARGET          = desktopfile-model

QT              += core gui widgets

INCLUDEPATH     = $$PWD/../../core/

LIBS            += -L$$OUT_PWD/../../core/ -lgraceful

SOURCES += \
    main.cpp



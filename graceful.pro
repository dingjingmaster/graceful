TEMPLATE = lib
TARGET = graceful

INSTALL_HEADER_DIR = /usr/include/graceful/


include($$PWD/SingleApplication/singleapplication.pri)

target.path = /lib/


INSTALLS += \
    target                                      \
    SINGLE_LIB SINGLE_LIB_PRIV                  \

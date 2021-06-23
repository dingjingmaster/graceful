SOURCES += \
    $$PWD/Logger.cpp                            \
    $$PWD/FileAppender.cpp                      \
    $$PWD/ConsoleAppender.cpp                   \
    $$PWD/AbstractAppender.cpp                  \
    $$PWD/RollingFileAppender.cpp               \
    $$PWD/AbstractStringAppender.cpp            \


LOG_HEADERS += \
    $$PWD/Logger.h                              \
    $$PWD/FileAppender.h                        \
    $$PWD/ConsoleAppender.h                     \
    $$PWD/AbstractAppender.h                    \
    $$PWD/CuteLogger_global.h                   \
    $$PWD/RollingFileAppender.h                 \
    $$PWD/AbstractStringAppender.h              \


HEADERS += $$LOG_HEADERS


win32 {
    SOURCES += src/OutputDebugAppender.cpp
    HEADERS += include/OutputDebugAppender.h
}


android {
    SOURCES += src/AndroidAppender.cpp
    HEADERS += include/AndroidAppender.h
}

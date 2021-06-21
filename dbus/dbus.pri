HEADERS += \
    $$PWD/notifications_interface.h                         \


SOURCES += \
    $$PWD/notifications_interface.cpp                       \


OTHER_FILES += \
    $$PWD/org.freedesktop.Notifications.xml                 \
    $$PWD/org.graceful.SingleApplication.xml                \


#system(qdbusxml2cpp -m -p notifications_interface $$PWD/org.freedesktop.Notifications.xml)


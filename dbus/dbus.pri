


OTHER_FILES += \
    $$PWD/org.freedesktop.Notifications.xml                 \
    $$PWD/org.graceful.SingleApplication.xml                \


system(qdbusxml2cpp -m -p notifications_interface $$PWD/dbus/org.freedesktop.Notifications.xml)


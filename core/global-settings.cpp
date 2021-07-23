#include "global-settings.h"
#include "globals.h"
#include "qgsettings/QGSettings/QGSettings"

#include <QMutex>

graceful::GlobalSettings::GlobalSettings(QObject *parent) : QObject(parent)
{
    mGsettings = new QGSettings(GRACEFUL_SETTINGS_DAEMON, QByteArray(), this);
}

graceful::GlobalSettings *graceful::GlobalSettings::getInstance()
{
    static QMutex mutex;
    static GlobalSettings* gInstance = nullptr;
    if (!gInstance) {
        mutex.lock();
        gInstance = gInstance ? gInstance : new GlobalSettings;
        mutex.unlock();
    }

    return gInstance;
}

void graceful::GlobalSettings::onValueChanged(QString key)
{
    if (GRACEFUL_SETTINGS_ICON_THEME == key) {
        Q_EMIT iconThemeChanged();
    }
}

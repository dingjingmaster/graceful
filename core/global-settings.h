#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include <QObject>

class QGSettings;
namespace graceful
{
class GlobalSettings : public QObject
{
    Q_OBJECT
public:
    static GlobalSettings* getInstance();

Q_SIGNALS:
    void iconThemeChanged();

private Q_SLOTS:
    void onValueChanged(QString);

private:
    explicit GlobalSettings(QObject *parent = nullptr);

private:
    QGSettings*                     mGsettings = nullptr;
};
}

#endif // GLOBALSETTINGS_H

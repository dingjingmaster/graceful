#ifndef NOTIFICATION_P_H
#define NOTIFICATION_P_H

#include "notification.h"
#include "dbus/notifications_interface.h"


namespace graceful
{
class NotificationPrivate : public QObject
{
    Q_OBJECT
public:
    NotificationPrivate(const QString& summary, Notification* parent);
    ~NotificationPrivate() override;

    void close();
    void update();
    void queryServerInfo(bool async=1);
    const Notification::ServerInfo serverInfo();
    void setActions(QStringList actions, int defaultAction);

public Q_SLOTS:
    void notificationClosed(uint, uint);
    void handleAction(uint id, const QString& key);

private:
    uint                                    mId;

    QString                                 mSummary;
    QString                                 mBody;
    QString                                 mIconName;
    QStringList                             mActions;
    QVariantMap                             mHints;
    int                                     mDefaultAction;
    int                                     mTimeout;

    static Notification::ServerInfo         sServerInfo;
    static bool                             sIsServerInfoQuried;

    Notification* const                     q_ptr;
    OrgFreedesktopNotificationsInterface*   mInterface;

    Q_DECLARE_PUBLIC(Notification)
};
}


#endif // NOTIFICATION_P_H

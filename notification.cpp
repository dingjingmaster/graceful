#include "notification.h"

#include <QDebug>
#include <QMessageBox>

#include "notification_p.h"


using namespace graceful;


Notification::Notification(const QString& summary, QObject* parent) : QObject(parent), d_ptr(new NotificationPrivate(summary, this))
{

}

Notification::~Notification()
{
    Q_D(Notification);
    delete d;
}

void Notification::update()
{
    Q_D(Notification);
    d->update();
}

void Notification::close()
{
    Q_D(Notification);
    d->close();
}

void Notification::setSummary(const QString& summary)
{
    Q_D(Notification);
    d->mSummary = summary;
}

void Notification::setBody(const QString& body)
{
    Q_D(Notification);
    d->mBody = body;
}

void Notification::setIcon(const QString& iconName)
{
    Q_D(Notification);
    d->mIconName = iconName;
}

void Notification::setActions(const QStringList& actions, int defaultAction)
{
    Q_D(Notification);
    d->setActions(actions, defaultAction);
}

void Notification::setTimeout(int timeout)
{
    Q_D(Notification);
    d->mTimeout = timeout;
}

void Notification::setHint(const QString& hintName, const QVariant& value)
{
    Q_D(Notification);
    d->mHints.insert(hintName, value);
}

void Notification::setUrgencyHint(Urgency urgency)
{
    Q_D(Notification);
    d->mHints.insert("urgency", qvariant_cast<uchar>(urgency));
}

void Notification::clearHints()
{
    Q_D(Notification);
    d->mHints.clear();
}

QStringList Notification::getCapabilities()
{
    Q_D(Notification);
    return d->mInterface->GetCapabilities().value();
}

const Notification::ServerInfo Notification::serverInfo()
{
    Q_D(Notification);
    return d->serverInfo();
}

void Notification::queryServerInfo()
{
    Q_D(Notification);
    d->queryServerInfo(/*async=*/true);
}

void Notification::notify(const QString& summary, const QString& body, const QString& iconName)
{
    Notification notification(summary);
    notification.setBody(body);
    notification.setIcon(iconName);
    notification.update();
}

bool NotificationPrivate::sIsServerInfoQuried = 0;
Notification::ServerInfo NotificationPrivate::sServerInfo;

NotificationPrivate::NotificationPrivate(const QString& summary, Notification* parent) : mId(0), mSummary(summary), mTimeout(-1), q_ptr(parent)
{
    mInterface = new OrgFreedesktopNotificationsInterface("org.freedesktop.Notifications", "/org/freedesktop/Notifications", QDBusConnection::sessionBus(), this);
    connect(mInterface, &OrgFreedesktopNotificationsInterface::NotificationClosed, this, &NotificationPrivate::notificationClosed);
    connect(mInterface, &OrgFreedesktopNotificationsInterface::ActionInvoked, this, &NotificationPrivate::handleAction);
}

NotificationPrivate::~NotificationPrivate() = default;

void NotificationPrivate::update()
{
    QDBusPendingReply<uint> reply = mInterface->Notify(qAppName(), mId, mIconName, mSummary, mBody, mActions, mHints, mTimeout);
    reply.waitForFinished();
    if (!reply.isError()) {
        mId = reply.value();
    } else {
        if (mHints.contains("urgency") && mHints.value("urgency").toInt() != Notification::UrgencyLow)
            QMessageBox::information(nullptr, tr("Notifications Fallback"), mSummary + " \n\n " + mBody);
    }
}


void NotificationPrivate::setActions(QStringList actions, int defaultAction)
{
    mActions.clear();
    mDefaultAction = defaultAction;
    const int N = actions.size();
    for (int ix = 0; ix < N; ix++) {
        if (ix == defaultAction) {
            mActions.append("default");
        } else {
            mActions.append(QString::number(ix));
        }
        mActions.append(actions[ix]);
    }
}

const Notification::ServerInfo NotificationPrivate::serverInfo()
{
    if (!sIsServerInfoQuried) {
        queryServerInfo (/*async=*/false);
    }

    return sServerInfo;
}

void NotificationPrivate::queryServerInfo(bool async)
{
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(mInterface->GetServerInformation(), this);

    connect(watcher, &QDBusPendingCallWatcher::finished, [this](QDBusPendingCallWatcher* callWatcher) {
        Q_Q(Notification);
        QDBusPendingReply<QString, QString, QString, QString> reply = *callWatcher;

        if (!reply.isError()) {
            sServerInfo.name         = reply.argumentAt<0>();
            sServerInfo.vendor       = reply.argumentAt<1>();
            sServerInfo.version      = reply.argumentAt<2>();
            sServerInfo.specVersion  = reply.argumentAt<3>();
        } else {
            // server either responds something strange or nothing; assume it's malfunctioning
            sServerInfo.name.clear();
            sServerInfo.vendor.clear();
            sServerInfo.version.clear();
            sServerInfo.specVersion.clear();
        }
        sIsServerInfoQuried = true;
        Q_EMIT q->serverInfoReady();
        sender()->deleteLater();
    });

    if (!async) {
        QEventLoop loop;
        connect(watcher, &QDBusPendingCallWatcher::finished, &loop, &QEventLoop::quit);
        loop.exec();
    }
}

void NotificationPrivate::handleAction(uint id, const QString& key)
{
    if (id != mId)
        return;

    Q_Q(Notification);
    qDebug() << "action invoked:" << key;
    bool ok = true;
    int keyId;
    if (key == "default") {
        keyId = mDefaultAction;
    } else {
        keyId = key.toInt(&ok);
    }

    if (ok && keyId >= 0)
        Q_EMIT q->actionActivated(keyId);
}

void NotificationPrivate::close()
{
    mInterface->CloseNotification(mId);
    mId = 0;
}

void NotificationPrivate::notificationClosed(uint id, uint reason)
{
    Q_Q(Notification);
    if (id != 0 && id == mId) {
        mId = 0;
    }

    Q_EMIT q->notificationClosed(Notification::CloseReason(reason));
}


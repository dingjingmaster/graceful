#include "power-provider.h"


#include <QDebug>
#include <csignal>
#include <QProcess>
#include <QDBusInterface>

#include "notification.h"


#define UPOWER_SERVICE              "org.freedesktop.UPower"
#define UPOWER_PATH                 "/org/freedesktop/UPower"
#define UPOWER_INTERFACE            UPOWER_SERVICE

#define CONSOLEKIT_SERVICE          "org.freedesktop.ConsoleKit"
#define CONSOLEKIT_PATH             "/org/freedesktop/ConsoleKit/Manager"
#define CONSOLEKIT_INTERFACE        "org.freedesktop.ConsoleKit.Manager"

#define SYSTEMD_SERVICE             "org.freedesktop.login1"
#define SYSTEMD_PATH                "/org/freedesktop/login1"
#define SYSTEMD_INTERFACE           "org.freedesktop.login1.Manager"

#define GRACEFUL_SERVICE            "org.graceful.session"
#define GRACEFUL_PATH               "/GracefulSession"
#define GRACEFUL_INTERFACE          "org.graceful.session"

#define PROPERTIES_INTERFACE        "org.freedesktop.DBus.Properties"

using namespace graceful;

void printDBusMsg(const QDBusMessage &msg)
{
    qWarning() << "** Dbus error **************************";
    qWarning() << "Error name " << msg.errorName();
    qWarning() << "Error msg  " << msg.errorMessage();
    qWarning() << "****************************************";
}

static bool dbusCall(const QString &service,
                     const QString &path,
                     const QString &interface,
                     const QDBusConnection &connection,
                     const QString & method,
                     PowerProvider::DbusErrorCheck errorCheck = PowerProvider::CheckDBUS)
{
    QDBusInterface dbus(service, path, interface, connection);
    if (!dbus.isValid()) {
        qWarning() << "dbusCall: QDBusInterface is invalid" << service << path << interface << method;
        if (errorCheck == PowerProvider::CheckDBUS) {
            Notification::notify(
                QObject::tr("Power Manager Error"),
                QObject::tr("QDBusInterface is invalid") + "\n\n" + service + ' ' + path + ' ' + interface + ' ' + method,
                "xxx.png");
        }
        return false;
    }

    QDBusMessage msg = dbus.call(method);

    if (!msg.errorName().isEmpty()) {
        printDBusMsg(msg);
        if (errorCheck == PowerProvider::CheckDBUS) {
            Notification::notify(
                QObject::tr("Power Manager Error (D-BUS call)"),
                msg.errorName() + "\n\n" + msg.errorMessage(),
                "xxx.png");
        }
    }

    // If the method no returns value, we believe that it was successful.
    return msg.arguments().isEmpty() ||
           msg.arguments().constFirst().isNull() ||
           msg.arguments().constFirst().toBool();
}

static bool dbusCallSystemd(const QString &service,
                            const QString &path,
                            const QString &interface,
                            const QDBusConnection &connection,
                            const QString &method,
                            bool needBoolArg,
                            PowerProvider::DbusErrorCheck errorCheck = PowerProvider::CheckDBUS)
{
    QDBusInterface dbus(service, path, interface, connection);
    if (!dbus.isValid()) {
        qWarning() << "dbusCall: QDBusInterface is invalid" << service << path << interface << method;
        if (errorCheck == PowerProvider::CheckDBUS) {
            Notification::notify(
                QObject::tr("Power Manager Error"),
                QObject::tr("QDBusInterface is invalid") + "\n\n" + service + ' ' + path + ' '+ interface + ' ' + method,
                "xxx.png");
        }
        return false;
    }

    QDBusMessage msg = needBoolArg ? dbus.call(method, QVariant(true)) : dbus.call(method);

    if (!msg.errorName().isEmpty()) {
        printDBusMsg(msg);
        if (errorCheck == PowerProvider::CheckDBUS) {
            Notification::notify(
                QObject::tr("Power Manager Error (D-BUS call)"),
                msg.errorName() + "\n\n" + msg.errorMessage(),
                "xxx.png");
        }
    }

    // If the method no returns value, we believe that it was successful.
    if (msg.arguments().isEmpty() || msg.arguments().constFirst().isNull()) {
        return true;
    }

    QString response = msg.arguments().constFirst().toString();
    qDebug() << "systemd:" << method << "=" << response;
    return response == "yes" || response == "challenge";
}


bool dbusGetProperty(const QString &service,
                     const QString &path,
                     const QString &interface,
                     const QDBusConnection &connection,
                     const QString & property)
{
    QDBusInterface dbus(service, path, interface, connection);
    if (!dbus.isValid()) {
        qWarning() << "dbusGetProperty: QDBusInterface is invalid" << service << path << interface << property;
        //        Notification::notify(QObject::tr("Graceful Power Manager"),
        //                                  "xxx-logo.png",
        //                                  QObject::tr("Power Manager Error"),
        //                                  QObject::tr("QDBusInterface is invalid")+ "\n\n" + service +" " + path +" " + interface +" " + property);

        return false;
    }

    QDBusMessage msg = dbus.call("Get", dbus.interface(), property);

    if (!msg.errorName().isEmpty()) {
        printDBusMsg(msg);
        //        Notification::notify(QObject::tr("Graceful Power Manager"),
        //                                  "xxx-logo.png",
        //                                  QObject::tr("Power Manager Error (Get Property)"),
        //                                  msg.errorName() + "\n\n" + msg.errorMessage());
    }

    return !msg.arguments().isEmpty() && msg.arguments().constFirst().value<QDBusVariant>().variant().toBool();
}



PowerProvider::PowerProvider(QObject *parent) : QObject(parent)
{

}


PowerProvider::~PowerProvider() = default;


UPowerProvider::UPowerProvider(QObject *parent) : PowerProvider(parent)
{

}


UPowerProvider::~UPowerProvider() = default;


bool UPowerProvider::canAction(Power::Action action) const
{
    QString command;
    QString property;
    switch (action)
    {
    case Power::PowerHibernate:
        property = "CanHibernate";
        command  = "HibernateAllowed";
        break;

    case Power::PowerSuspend:
        property = "CanSuspend";
        command  = "SuspendAllowed";
        break;

    default:
        return false;
    }

    return dbusGetProperty(UPOWER_SERVICE, UPOWER_PATH, PROPERTIES_INTERFACE, QDBusConnection::systemBus(), property)
           && dbusCall(UPOWER_SERVICE, UPOWER_PATH, UPOWER_INTERFACE, QDBusConnection::systemBus(), command, PowerProvider::DontCheckDBUS);
}


bool UPowerProvider::doAction(Power::Action action)
{
    QString command;

    switch (action)
    {
    case Power::PowerHibernate:
        command = "Hibernate";
        break;

    case Power::PowerSuspend:
        command = "Suspend";
        break;

    default:
        return false;
    }


    return dbusCall(UPOWER_SERVICE, UPOWER_PATH, UPOWER_INTERFACE, QDBusConnection::systemBus(), command);
}


ConsoleKitProvider::ConsoleKitProvider(QObject *parent) : PowerProvider(parent)
{

}


ConsoleKitProvider::~ConsoleKitProvider() = default;


bool ConsoleKitProvider::canAction(Power::Action action) const
{
    QString command;
    switch (action)
    {
    case Power::PowerReboot:
        command = "CanReboot";
        break;

    case Power::PowerShutdown:
        command = "CanPowerOff";
        break;

    case Power::PowerHibernate:
        command  = "CanHibernate";
        break;

    case Power::PowerSuspend:
        command  = "CanSuspend";
        break;

    default:
        return false;
    }

    return dbusCallSystemd(CONSOLEKIT_SERVICE, CONSOLEKIT_PATH, CONSOLEKIT_INTERFACE, QDBusConnection::systemBus(), command, false, PowerProvider::DontCheckDBUS);
}


bool ConsoleKitProvider::doAction(Power::Action action)
{
    QString command;
    switch (action)
    {
    case Power::PowerReboot:
        command = "Reboot";
        break;

    case Power::PowerShutdown:
        command = "PowerOff";
        break;

    case Power::PowerHibernate:
        command = "Hibernate";
        break;

    case Power::PowerSuspend:
        command = "Suspend";
        break;

    default:
        return false;
    }

    return dbusCallSystemd(CONSOLEKIT_SERVICE, CONSOLEKIT_PATH, CONSOLEKIT_INTERFACE, QDBusConnection::systemBus(), command, true);
}

SystemdProvider::SystemdProvider(QObject *parent) : PowerProvider(parent)
{

}


SystemdProvider::~SystemdProvider() = default;


bool SystemdProvider::canAction(Power::Action action) const
{
    QString command;

    switch (action)
    {
    case Power::PowerReboot:
        command = "CanReboot";
        break;

    case Power::PowerShutdown:
        command = "CanPowerOff";
        break;

    case Power::PowerSuspend:
        command = "CanSuspend";
        break;

    case Power::PowerHibernate:
        command = "CanHibernate";
        break;

    default:
        return false;
    }

    return dbusCallSystemd(SYSTEMD_SERVICE, SYSTEMD_PATH, SYSTEMD_INTERFACE, QDBusConnection::systemBus(), command, false, PowerProvider::DontCheckDBUS);
}


bool SystemdProvider::doAction(Power::Action action)
{
    QString command;

    switch (action)
    {
    case Power::PowerReboot:
        command = "Reboot";
        break;

    case Power::PowerShutdown:
        command = "PowerOff";
        break;

    case Power::PowerSuspend:
        command = "Suspend";
        break;

    case Power::PowerHibernate:
        command = "Hibernate";
        break;

    default:
        return false;
    }

    return dbusCallSystemd(SYSTEMD_SERVICE, SYSTEMD_PATH, SYSTEMD_INTERFACE, QDBusConnection::systemBus(), command, true);
}


GracefulProvider::GracefulProvider(QObject *parent) : PowerProvider(parent)
{

}


GracefulProvider::~GracefulProvider() = default;


bool GracefulProvider::canAction(Power::Action action) const
{
    QString command;
    switch (action)
    {
    case Power::PowerLogout:
        command = "canLogout";
        break;
    case Power::PowerReboot:
        command = "canReboot";
        break;
    case Power::PowerShutdown:
        command = "canPowerOff";
        break;
    default:
        return false;
    }

    // there can be case when gracefulsession-session does not run
    return dbusCall(GRACEFUL_SERVICE, GRACEFUL_PATH, GRACEFUL_INTERFACE, QDBusConnection::sessionBus(), command, PowerProvider::DontCheckDBUS);
}


bool GracefulProvider::doAction(Power::Action action)
{
    QString command;
    switch (action)
    {
    case Power::PowerLogout:
        command = "logout";
        break;
    case Power::PowerReboot:
        command = "reboot";
        break;
    case Power::PowerShutdown:
        command = "powerOff";
        break;
    default:
        return false;
    }

    return dbusCall(GRACEFUL_SERVICE, GRACEFUL_PATH, GRACEFUL_INTERFACE, QDBusConnection::sessionBus(), command);
}

GracefulSessionProvider::GracefulSessionProvider(QObject *parent) : PowerProvider(parent)
{
    pid = (Q_PID)qgetenv("_LXSESSION_PID").toLong();
}


GracefulSessionProvider::~GracefulSessionProvider() = default;


bool GracefulSessionProvider::canAction(Power::Action action) const
{
    switch (action)
    {
    case Power::PowerLogout:
        return pid != 0;
    default:
        return false;
    }
}


bool GracefulSessionProvider::doAction(Power::Action action)
{
    switch (action)
    {
    case Power::PowerLogout:
        if(pid)
            ::kill(pid, SIGTERM);
        break;
    default:
        return false;
    }

    return true;
}


HalProvider::HalProvider(QObject *parent) : PowerProvider(parent)
{

}


HalProvider::~HalProvider() = default;


bool HalProvider::canAction(Power::Action action) const
{
    Q_UNUSED(action)
    return false;
}


bool HalProvider::doAction(Power::Action action)
{
    Q_UNUSED(action)
    return false;
}

CustomProvider::CustomProvider(QObject *parent) : PowerProvider(parent), mSettings("power")
{

}

CustomProvider::~CustomProvider() = default;

bool CustomProvider::canAction(Power::Action action) const
{
    switch (action)
    {
    case Power::PowerShutdown:
        return mSettings.contains("shutdownCommand");

    case Power::PowerReboot:
        return mSettings.contains("rebootCommand");

    case Power::PowerHibernate:
        return mSettings.contains("hibernateCommand");

    case Power::PowerSuspend:
        return mSettings.contains("suspendCommand");

    case Power::PowerLogout:
        return mSettings.contains("logoutCommand");

    case Power::PowerMonitorOff:
        return mSettings.contains("monitorOffCommand");

    case Power::PowerShowLeaveDialog:
        return mSettings.contains("showLeaveDialogCommand");

    default:
        return false;
    }
}

bool CustomProvider::doAction(Power::Action action)
{
    QString command;

    switch(action)
    {
    case Power::PowerShutdown:
        command = mSettings.value("shutdownCommand").toString();
        break;

    case Power::PowerReboot:
        command = mSettings.value("rebootCommand").toString();
        break;

    case Power::PowerHibernate:
        command = mSettings.value("hibernateCommand").toString();
        break;

    case Power::PowerSuspend:
        command = mSettings.value("suspendCommand").toString();
        break;

    case Power::PowerLogout:
        command = mSettings.value("logoutCommand").toString();
        break;

    case Power::PowerMonitorOff:
        command = mSettings.value("monitorOffCommand").toString();
        break;

    case Power::PowerShowLeaveDialog:
        command = mSettings.value("showLeaveDialogCommand").toString();
        break;

    default:
        return false;
    }

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    return QProcess::startDetached(command);
#else
    QStringList args = QProcess::splitCommand(command);
    if (args.isEmpty())
        return false;

    QProcess process;
    process.setProgram(args.takeFirst());
    process.setArguments(args);
    return process.startDetached();
#endif
}


#ifndef POWER_H
#define POWER_H

#include <QList>
#include <QObject>

#include "globals.h"


namespace graceful
{
class PowerProvider;

class GRACEFUL_API Power : public QObject
{
    Q_OBJECT
public:
    /// Power can perform next actions:
    enum Action {
        PowerLogout,    /// Close the current user session.
        PowerHibernate, /// Hibernate the comupter
        PowerReboot,    /// Reboot the computer
        PowerShutdown,  /// Shutdown the computer
        PowerSuspend,   /// Suspend the computer
        PowerMonitorOff, /// Turn off the monitor(s)
        PowerShowLeaveDialog /// Show the leave dialog
    };

    /*!
     * Constructs the Power object.
     * \param usegracefulSessionProvider indicates if the DBus methods
     * provided by graceful-session should be considered. This is useful to
     * avoid recursion if the graceful-session wants to provide some of the
     * methods by itself with internal use of this object.
     */
    explicit Power(bool useGracefulSessionProvider, QObject *parent = nullptr);
    /// Constructs a Power with using the graceful-session provider.
    explicit Power(QObject *parent = nullptr);

    /// Destroys the object.
    ~Power() override;

    /// Returns true if the Power can perform action.
    bool canAction(Action action) const;

    //! This function is provided for convenience. It's equivalent to calling canAction(PowerLogout).
    bool canLogout() const;

    //! This function is provided for convenience. It's equivalent to calling canAction(PowerHibernate).
    bool canHibernate() const;

    //! This function is provided for convenience. It's equivalent to calling canAction(PowerReboot).
    bool canReboot() const;

    //! This function is provided for convenience. It's equivalent to calling canAction(PowerShutdown).
    bool canShutdown() const;

    //! This function is provided for convenience. It's equivalent to calling canAction(PowerSuspend).
    bool canSuspend() const;

    //! This function is provided for convenience. It's equivalent to calling canAction(PowerMonitorOff).
    bool canMonitorOff() const;

    //! This function is provided for convenience. It's equivalent to calling canAction(PowerShowLeaveDialog).
    bool canShowLeaveDialog() const;

public Q_SLOTS:
    /// Performs the requested action.
    bool doAction(Action action);

    //! This function is provided for convenience. It's equivalent to calling doAction(PowerLogout).
    bool logout();

    //! This function is provided for convenience. It's equivalent to calling doAction(PowerHibernate).
    bool hibernate();

    //! This function is provided for convenience. It's equivalent to calling doAction(PowerReboot).
    bool reboot();

    //! This function is provided for convenience. It's equivalent to calling doAction(PowerShutdown).
    bool shutdown();

    //! This function is provided for convenience. It's equivalent to calling doAction(PowerSuspend).
    bool suspend();

    //! This function is provided for convenience. It's equivalent to calling doAction(PowerMonitorOff).
    bool monitorOff();

    //! This function is provided for convenience. It's equivalent to calling doAction(PowerShowLeaveDialog).
    bool showLeaveDialog();

private:
    QList<PowerProvider*>       mProviders;
};


}

#endif // POWER_H

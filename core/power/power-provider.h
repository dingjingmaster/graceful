#ifndef POWERPROVIDER_H
#define POWERPROVIDER_H

#include <QObject>
#include <QProcess>

#include "globals.h"
#include "settings.h"
#include "power/power.h"


namespace graceful
{

class PowerProvider: public QObject
{
    Q_OBJECT
public:

    enum DbusErrorCheck {
        CheckDBUS,
        DontCheckDBUS
    };

    explicit PowerProvider(QObject *parent = nullptr);
    ~PowerProvider() override;

    /*! Returns true if the Power can perform action.
        This is a pure virtual function, and must be reimplemented in subclasses. */
    virtual bool canAction(Power::Action action) const = 0 ;

public Q_SLOTS:
    /*! Performs the requested action.
        This is a pure virtual function, and must be reimplemented in subclasses. */
    virtual bool doAction(Power::Action action) = 0;
};


class UPowerProvider: public PowerProvider
{
    Q_OBJECT
public:
    UPowerProvider(QObject *parent = nullptr);
    ~UPowerProvider() override;
    bool canAction(Power::Action action) const override;

public Q_SLOTS:
    bool doAction(Power::Action action) override;
};


class ConsoleKitProvider: public PowerProvider
{
    Q_OBJECT
public:
    ConsoleKitProvider(QObject *parent = nullptr);
    ~ConsoleKitProvider() override;
    bool canAction(Power::Action action) const override;

public Q_SLOTS:
    bool doAction(Power::Action action) override;
};


class SystemdProvider: public PowerProvider
{
    Q_OBJECT
public:
    SystemdProvider(QObject *parent = nullptr);
    ~SystemdProvider() override;
    bool canAction(Power::Action action) const override;

public Q_SLOTS:
    bool doAction(Power::Action action) override;
};


class GracefulProvider: public PowerProvider
{
    Q_OBJECT
public:
    GracefulProvider(QObject *parent = nullptr);
    ~GracefulProvider() override;
    bool canAction(Power::Action action) const override;

public Q_SLOTS:
    bool doAction(Power::Action action) override;
};

class GracefulSessionProvider: public PowerProvider
{
    Q_OBJECT
public:
    GracefulSessionProvider(QObject *parent = nullptr);
    ~GracefulSessionProvider() override;
    bool canAction(Power::Action action) const override;

public Q_SLOTS:
    bool doAction(Power::Action action) override;
private:
    Q_PID pid;
};

class HalProvider: public PowerProvider
{
    Q_OBJECT
public:
    HalProvider(QObject *parent = nullptr);
    ~HalProvider() override;
    bool canAction(Power::Action action) const override;

public Q_SLOTS:
    bool doAction(Power::Action action) override;
};


class CustomProvider: public PowerProvider
{
    Q_OBJECT
public:
    CustomProvider(QObject *parent = nullptr);
    ~CustomProvider() override;
    bool canAction(Power::Action action) const override;

public Q_SLOTS:
    bool doAction(Power::Action action) override;

private:
    Settings                mSettings;
};


}

#endif // POWERPROVIDER_H

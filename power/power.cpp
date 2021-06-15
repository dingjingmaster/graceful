#include "power.h"

#include <QDebug>
#include <QtAlgorithms>

#include "power-provider.h"


using namespace graceful;


Power::Power(bool usegracefulSessionProvider, QObject * parent /*= nullptr*/) :
    QObject(parent)
{
    mProviders.append(new CustomProvider(this));
    if (usegracefulSessionProvider) {
        mProviders.append(new GracefulProvider(this));
    }
    mProviders.append(new SystemdProvider(this));
    mProviders.append(new UPowerProvider(this));
    mProviders.append(new ConsoleKitProvider(this));
    mProviders.append(new GracefulSessionProvider(this));
}

Power::Power(QObject * parent /*= nullptr*/)
    : Power(true, parent)
{

}


Power::~Power() = default;


bool Power::canAction(Power::Action action) const
{
    for(const PowerProvider* provider : qAsConst(mProviders))
    {
        if (provider->canAction(action))
            return true;
    }

    return false;
}


bool Power::doAction(Power::Action action)
{
    for(PowerProvider* provider : qAsConst(mProviders))
    {
        if (provider->canAction(action) &&
            provider->doAction(action)
            )
        {
            return true;
        }
    }
    return false;
}


bool Power::canLogout()             const { return canAction(PowerLogout);          }
bool Power::canHibernate()          const { return canAction(PowerHibernate);       }
bool Power::canReboot()             const { return canAction(PowerReboot);          }
bool Power::canShutdown()           const { return canAction(PowerShutdown);        }
bool Power::canSuspend()            const { return canAction(PowerSuspend);         }
bool Power::canMonitorOff()         const { return canAction(PowerMonitorOff);      }
bool Power::canShowLeaveDialog()    const { return canAction(PowerShowLeaveDialog); }

bool Power::logout()            { return doAction(PowerLogout);         }
bool Power::hibernate()         { return doAction(PowerHibernate);      }
bool Power::reboot()            { return doAction(PowerReboot);         }
bool Power::shutdown()          { return doAction(PowerShutdown);       }
bool Power::suspend()           { return doAction(PowerSuspend);        }
bool Power::monitorOff()        { return doAction(PowerMonitorOff);     }
bool Power::showLeaveDialog()   { return doAction(PowerShowLeaveDialog);}

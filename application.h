#ifndef APPLICATION_H
#define APPLICATION_H

#include <QDir>
#include <QApplication>
#include <QProxyStyle>

#include "globals.h"

namespace graceful
{

class GRACEFUL_API Application : public QApplication
{
    Q_OBJECT

public:
    Application(int &argc, char **argv);
    Application(int &argc, char **argv, bool handleQuitSignals);
    ~Application() override {}
    void listenToUnixSignals(QList<int> const & signolList);

private Q_SLOTS:
    void updateTheme();

Q_SIGNALS:
    void themeChanged();
    void unixSignal(int signo);
};

#if defined(gracefulApp)
#undef gracefulApp
#endif
#define gracefulApp (static_cast<graceful::Application *>(qApp))
}

#endif // APPLICATION_H

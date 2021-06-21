#include "application.h"

#include "settings.h"

#include <XdgDirs>

using namespace graceful;

#define COLOR_DEBUG     "\033[32;2m"
#define COLOR_WARN      "\033[33;2m"
#define COLOR_CRITICAL  "\033[31;1m"
#define COLOR_FATAL     "\033[33;1m"
#define COLOR_RESET     "\033[0m"

#define QAPP_NAME qApp ? qApp->objectName().toUtf8().constData() : ""

#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <csignal>
#include <cerrno>
#include <sys/socket.h>
#include <QDateTime>
#include <QDebug>
#include <QSocketNotifier>

Application::Application(int &argc, char** argv) : QApplication(argc, argv)
{
    setWindowIcon(QIcon(QFile::decodeName(GRACEFUL_GRAPHICS_DIR) + QL1S("/xxx.png")));
    connect(Settings::globalSettings(), &GlobalSettings::gracefulThemeChanged, this, &Application::updateTheme);
    updateTheme();
}

Application::Application(int &argc, char** argv, bool handleQuitSignals) : Application(argc, argv)
{
    if (handleQuitSignals) {
        QList<int> signo_list = {SIGINT, SIGTERM, SIGHUP};
        connect(this, &Application::unixSignal, [this, signo_list] (int signo) {
            if (signo_list.contains(signo))
                quit();
        });
        listenToUnixSignals(signo_list);
    }
}

void Application::updateTheme()
{
    const QString styleSheetKey = QFileInfo(applicationFilePath()).fileName();
    setStyleSheet(gracefulTheme.qss(styleSheetKey));
    Q_EMIT themeChanged();
}

namespace
{
class SignalHandler
{
public:
    static void signalHandler(int signo)
    {
        const int ret = write(instance->mSignalSock[0], &signo, sizeof (int));
        if (sizeof (int) != ret)
            qCritical("unable to write into socketpair: %s", strerror(errno));
    }

public:
    template <class Lambda>
    SignalHandler(Application * app, Lambda signalEmitter)
        : mSignalSock{-1, -1}
    {
        if (0 != socketpair(AF_UNIX, SOCK_STREAM, 0, mSignalSock))
        {
            qCritical("unable to create socketpair for correct signal handling: %s", strerror(errno));
            return;
        }

        mNotifier.reset(new QSocketNotifier(mSignalSock[1], QSocketNotifier::Read));
        QObject::connect(mNotifier.data(), &QSocketNotifier::activated, app, [this, signalEmitter] {
            int signo = 0;
            int ret = read(mSignalSock[1], &signo, sizeof (int));
            if (sizeof (int) != ret)
                qCritical("unable to read signal from socketpair, %s", strerror(errno));
            signalEmitter(signo);
        });
    }

    ~SignalHandler()
    {
        close(mSignalSock[0]);
        close(mSignalSock[1]);
    }

    void listenToSignals(QList<int> const & signoList)
    {
        struct sigaction sa;
        sa.sa_handler = signalHandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        for (auto const & signo : signoList)
            sigaction(signo, &sa, nullptr);
    }

public:
    static QScopedPointer<SignalHandler> instance;

private:
    int mSignalSock[2];
    QScopedPointer<QSocketNotifier> mNotifier;
};

QScopedPointer<SignalHandler> SignalHandler::instance;
}

void Application::listenToUnixSignals(QList<int> const & signoList)
{
    static QScopedPointer<QSocketNotifier> signal_notifier;

    if (SignalHandler::instance.isNull())
        SignalHandler::instance.reset(new SignalHandler{this, [this] (int signo) { Q_EMIT unixSignal(signo); }});
    SignalHandler::instance->listenToSignals(signoList);
}


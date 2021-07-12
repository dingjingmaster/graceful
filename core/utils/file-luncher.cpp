#include "file-luncher.h"

#include "log/log.h"
#include "file/file.h"

#include <QUrl>
#include <QDebug>
#include <QProcess>

void graceful::FileLuncher::openFileByUri(QString uri)
{
    File file(uri);

    g_return_if_fail(file.isValid());

    // get file manager

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    QProcess p;
    p.setProgram("nautilus");
    p.setArguments(QStringList() << file.uri());
    p.startDetached();
    log_debug("command:%s %s", "nautilus", file.uri().toUtf8().constData());
#else
    // else if fixme
#endif
}

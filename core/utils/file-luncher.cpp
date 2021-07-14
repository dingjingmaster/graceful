#include "file-luncher.h"

#include "log/log.h"
#include "file/file.h"
#include <gio/gdesktopappinfo.h>

#include <QUrl>
#include <QDebug>
#include <QProcess>

static void launchApp(graceful::File& f);
static void openFileByApp(graceful::File& f);
static void openFolderByFileManager(graceful::File& f);

void graceful::FileLuncher::openFileByUri(QString uri)
{
    File file(uri);

    gf_return_if_fail(file.isValid());

    // can execute
    if (file.canExecute()) {
        if (file.uriDisplay().endsWith(".desktop")) {
            launchApp(file);
        } else if (file.isDir()) {
            openFolderByFileManager(file);
        } else {
            openFileByApp(file);
        }
    } else {
        openFileByApp(file);
    }
}

static void launchApp(graceful::File& f)
{
    GError* error = nullptr;

    g_autoptr(GDesktopAppInfo) desktopInfo = g_desktop_app_info_new_from_filename(f.path().toUtf8().constData());

    gf_return_if_fail(G_IS_DESKTOP_APP_INFO(desktopInfo));

    const char* cmd = g_app_info_get_executable(G_APP_INFO(desktopInfo));

    g_app_info_launch(G_APP_INFO(desktopInfo), nullptr, nullptr, &error);
    if (error) {
        log_error("open file: '%s' by '%s', error: %s", f.uriDisplay().toUtf8().constData(), cmd, error->message);
    }
}

static void openFileByApp(graceful::File& f)
{
    g_autoptr(GAppInfo) app = g_app_info_get_default_for_type(f.getContentType().toUtf8().constData(), true);
    if (!app) {
        app = g_app_info_get_default_for_uri_scheme (f.schema().toUtf8().constData());
    }

    gf_return_if_fail(G_IS_APP_INFO(app));

    GList* ls = nullptr;
    GError* error = nullptr;

    const char* cmd = g_app_info_get_executable(app);
    GFile* gfile = g_file_new_for_uri(f.uri().toUtf8().constData());

    ls = g_list_append(ls, gfile);

    g_app_info_launch(app, ls, nullptr, &error);
    if (error) {
        log_error("open file: '%s' by '%s', error: %s", f.uriDisplay().toUtf8().constData(), cmd, error->message);
    }

    g_list_free_full(ls, g_object_unref);
}

static void openFolderByFileManager(graceful::File& file)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    QProcess p;
    p.setProgram("nautilus");
    p.setArguments(QStringList() << "-s" << file.uriDisplay());
    p.startDetached();
    log_debug("command:%s %s", "nautilus", file.uriDisplay().toUtf8().constData());
#else
    // else if fixme
#endif
}

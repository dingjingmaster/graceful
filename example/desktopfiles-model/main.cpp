#include <QApplication>
#include <QGuiApplication>
#include <QMainWindow>

#include <QFileSystemModel>
#include "desktop-view/desktop-view.h"
#include "desktop-model/desktopfile-model.h"

int main (int argc, char* argv[])
{
    using namespace graceful;

    QApplication app(argc, argv);

    QWidget* mainWindow = new QWidget;
    mainWindow->setFixedSize(900, 900);
    DesktopView* view = new DesktopView(mainWindow);

#if 1
    DesktopFileModel* desktopFileModel = new DesktopFileModel;
    desktopFileModel->setRootPath(QDir::currentPath());
    view->setModel(desktopFileModel);
#else
    QFileSystemModel* model = new QFileSystemModel;
    model->setRootPath(QDir::currentPath());

    view->setModel(model);
#endif

    view->setMinimumSize(900, 900);
    mainWindow->show();

    return app.exec();
}

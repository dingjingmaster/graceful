#include <QApplication>
#include <QGuiApplication>
#include <QMainWindow>

#include <QFileSystemModel>
#include "icon-view/icon-view.h"
#include "file-model/file-model.h"

int main (int argc, char* argv[])
{
    using namespace graceful;

    QApplication app(argc, argv);

    QWidget* mainWindow = new QWidget;
    mainWindow->setFixedSize(900, 900);
    IconView* view = new IconView(mainWindow);

#if 1
    FileModel* fileModel = new FileModel;
    fileModel->setRootPath(QDir::currentPath());
    view->setModel(fileModel);
#else
    QFileSystemModel* model = new QFileSystemModel;
    model->setRootPath(QDir::currentPath());

    view->setModel(model);
#endif

    view->setMinimumSize(900, 900);
    mainWindow->show();

    return app.exec();
}

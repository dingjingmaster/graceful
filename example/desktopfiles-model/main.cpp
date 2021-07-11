#include <QApplication>
#include <QGuiApplication>
#include <QMainWindow>
#include <QTreeView>

#include <QFileSystemModel>
#include "desktopfiles-model/desktopfile-model.h"

int main (int argc, char* argv[])
{
    using namespace graceful;

    QApplication app(argc, argv);

    QWidget* mainWindow = new QWidget;
    mainWindow->setFixedSize(900, 900);
    QTreeView* tree = new QTreeView(mainWindow);

#if 1
    DesktopFileModel* desktopFileModel = new DesktopFileModel;
    desktopFileModel->setRootPath(QDir::currentPath());
    tree->setModel(desktopFileModel);
#else
    QFileSystemModel* model = new QFileSystemModel;
    model->setRootPath(QDir::currentPath());

    tree->setModel(model);
#endif

    tree->setMinimumSize(900, 900);
    mainWindow->show();

    return app.exec();
}

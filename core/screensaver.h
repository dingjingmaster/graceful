#ifndef SCREENSAVER_H
#define SCREENSAVER_H

#include <QObject>
#include <QAction>

#include "globals.h"


namespace graceful
{
class ScreenSaverPrivate;

class GRACEFUL_API ScreenSaver : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ScreenSaver)
    Q_DISABLE_COPY(ScreenSaver)

public:
    ScreenSaver(QObject * parent=nullptr);
    ~ScreenSaver() override;

    QList<QAction*> availableActions();

Q_SIGNALS:
    void activated();
    void done();
public Q_SLOTS:
    void lockScreen();

private:
    ScreenSaverPrivate* const d_ptr;
};

}



#endif // SCREENSAVER_H

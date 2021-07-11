#ifndef ICONVIEWEDITOR_H
#define ICONVIEWEDITOR_H

#include "globals.h"

#include <QTextEdit>
#include <qlineedit.h>

namespace graceful
{
class GRACEFUL_API IconViewEditor : public QTextEdit
{
    Q_OBJECT
public:
    explicit IconViewEditor(QWidget *parent = nullptr);
    ~IconViewEditor() override;

Q_SIGNALS:
    void returnPressed();

public Q_SLOTS:
    void minimalAdjust();

protected:
    void paintEvent(QPaintEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

private:
    QLineEdit*                      mStyledEdit = nullptr;
};
}

#endif // ICONVIEWEDITOR_H

#ifndef ICONVIEWTEXTHELPER_H
#define ICONVIEWTEXTHELPER_H

#include "globals.h"

#include <QStyleOption>

namespace graceful
{
class GRACEFUL_API IconViewTextHelper
{
public:
    static QSize getTextSizeForIndex(const QStyleOptionViewItem &option, const QModelIndex &index, int horizalMargin = 0, int maxLineCount = 0);
    static void paintText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, int textMaxHeight, int horizalMargin = 0, int maxLineCount = 0, bool useSystemPalette = true, const QColor &customColor = Qt::transparent);
};

}

#endif // ICONVIEWTEXTHELPER_H

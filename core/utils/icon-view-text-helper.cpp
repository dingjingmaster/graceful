#include "icon-view-text-helper.h"

#include <QPainter>
#include <QTextLayout>

QSize graceful::IconViewTextHelper::getTextSizeForIndex(const QStyleOptionViewItem &option, const QModelIndex &index, int horizalMargin, int maxLineCount)
{
    int fixedWidth = option.rect.width() - horizalMargin*2;
    QString text = option.text;
    QFont font = option.font;
    QFontMetrics fontMetrics = option.fontMetrics;
    int lineSpacing = fontMetrics.lineSpacing();
    int textHight = 0;

    QTextLayout textLayout(text, font);

    QTextOption opt;
    opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    opt.setAlignment(Qt::AlignHCenter);

    textLayout.setTextOption(opt);
    textLayout.beginLayout();

    while (true) {
        QTextLine line = textLayout.createLine();
        if (!line.isValid())
            break;

        line.setLineWidth(fixedWidth);
        textHight += lineSpacing;
    }

    textLayout.endLayout();
    if (maxLineCount > 0) {
        textHight = qMin(maxLineCount * lineSpacing, textHight);
    }

    return QSize(fixedWidth, textHight);
}

void graceful::IconViewTextHelper::paintText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, int textMaxHeight, int horizalMargin, int maxLineCount, bool useSystemPalette, const QColor &customColor)
{
    painter->save();
    painter->translate(horizalMargin, 0);

    if (useSystemPalette) {
        if (option.state.testFlag(QStyle::State_Selected)) {
            painter->setPen(option.palette.highlightedText().color());
        } else {
            painter->setPen(option.palette.text().color());
        }
    }

    if (customColor != Qt::transparent) {
        painter->setPen(customColor);
    }

    int lineCount = 0;

    QString text = option.text;
    QFont font = option.font;
    QFontMetrics fontMetrics = option.fontMetrics;
    int lineSpacing = fontMetrics.lineSpacing();

    QTextLayout textLayout(text, font);

    QTextOption opt;
    opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    opt.setAlignment(Qt::AlignHCenter);

    textLayout.setTextOption(opt);
    textLayout.beginLayout();

    int width = option.rect.width() - 2*horizalMargin;

    int y = 0;
    while (true) {
        QTextLine line = textLayout.createLine();
        if (!line.isValid()) {
            break;
        }

        line.setLineWidth(width);
        int nextLineY = y + lineSpacing;
        lineCount++;

        if (textMaxHeight >= nextLineY + lineSpacing && lineCount != maxLineCount) {
            line.draw(painter, QPoint(0, y));
            y = nextLineY;
        } else {
            QString lastLine = option.text.mid(line.textStart());
            QString elidedLastLine = fontMetrics.elidedText(lastLine, Qt::ElideRight, width);
            auto rect = QRect(horizalMargin, y, width, textMaxHeight);
            opt.setWrapMode(QTextOption::NoWrap);
            painter->drawText(rect, elidedLastLine, opt);
            line = textLayout.createLine();
            break;
        }
    }
    textLayout.endLayout();

    painter->restore();
}

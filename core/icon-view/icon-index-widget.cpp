#include "icon-index-widget.h"

#include "icon-view.h"
#include "icon-view-delegate.h"
#include "utils/icon-view-text-helper.h"

#include <QStyle>
#include <QTimer>
#include <QPainter>
#include <QTextEdit>
#include <QMouseEvent>
#include <QStyleHints>
#include <QTextOption>
#include <QApplication>

#include <QDebug>

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

using namespace graceful;

IconIndexWidget::IconIndexWidget(IconViewDelegate *delegate, const QStyleOptionViewItem &option, const QModelIndex &index, QWidget *parent) : QWidget(parent)
{
    setContentsMargins(0, 0, 0, 0);
    mOption = option;

    mIndex = index;
    mDelegate = delegate;

    mCurrentFont = QApplication::font();

    updateItem();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    connect(qApp, &QApplication::fontChanged, this, [=]() {
        mDelegate->getView()->setIndexWidget(mIndex, nullptr);
    });
#endif

    auto view = mDelegate->getView();
}

IconIndexWidget::~IconIndexWidget()
{

}

void IconIndexWidget::paintEvent(QPaintEvent *e)
{
    auto view = mDelegate->getView();
    auto visualRect = mDelegate->getView()->visualRect(mIndex);
    if (this->pos() != visualRect.topLeft()) {
        move(visualRect.topLeft());
        return;
    }

    if (!view->selectionModel()->selectedIndexes().contains(mIndex)) {
        this->close();
        return;
    }

    Q_UNUSED(e)
    QPainter p(this);
    auto bgColor = mOption.palette.highlight().color();
    p.save();
    p.setPen(Qt::transparent);
    bgColor.setAlpha(255 * 0.7);
    p.setBrush(bgColor);
    p.drawRoundedRect(this->rect(), 6, 6);
    p.restore();

    auto opt = mOption;
    auto iconSizeExcepted = mDelegate->getView()->iconSize();
    auto iconRect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, opt.widget);
    int y_delta = iconSizeExcepted.height() - iconRect.height();
    opt.rect.moveTo(opt.rect.x(), opt.rect.y() + y_delta);

    int maxTextHight = this->height() - iconSizeExcepted.height() - 10;

    // draw icon
    opt.text = nullptr;
    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, &p, mDelegate->getView());

    p.save();
    p.translate(0, 5 + mDelegate->getView()->iconSize().height() + 5);

    if (mElideText) {
        int  charWidth = opt.fontMetrics.averageCharWidth();
        mOption.text = opt.fontMetrics.elidedText(mOption.text, Qt::ElideRight, ELIDE_TEXT_LENGTH * charWidth);
    }
    p.save();

    auto expectedSize = IconViewTextHelper::getTextSizeForIndex(mOption, mIndex, 2);
    QPixmap pixmap(expectedSize);
    pixmap.fill(Qt::transparent);
    QPainter shadowPainter(&pixmap);
    shadowPainter.setPen(Qt::black);
    IconViewTextHelper::paintText(&shadowPainter, mOption, mIndex, maxTextHight, 2, 0, false, Qt::black);
    shadowPainter.end();

    QImage shadowImage(expectedSize + QSize(4, 4), QImage::Format_ARGB32_Premultiplied);
    shadowImage.fill(Qt::transparent);
    shadowPainter.begin(&shadowImage);
    shadowPainter.drawPixmap(2, 2, pixmap);
    qt_blurImage(shadowImage, 8, false, false);

    for (int x = 0; x < shadowImage.width(); x++) {
        for (int y = 0; y < shadowImage.height(); y++) {
            auto color = shadowImage.pixelColor(x, y);
            if (color.alpha() > 0) {
                color.setAlphaF(qMin(color.alphaF() * 1.5, 1.0));
                shadowImage.setPixelColor(x, y, color);
            }
        }
    }

    shadowPainter.end();

    p.translate(-3, -1);
    p.drawImage(0, 0, shadowImage);

    p.restore();

    // draw text
    p.setPen(mOption.palette.highlightedText().color());
    IconViewTextHelper::paintText(&p, mOption, mIndex, 9999, 2, 0, false);
    p.restore();

    bgColor.setAlpha(255*0.8);
    p.setPen(bgColor);
    p.drawRoundedRect(this->rect().adjusted(0, 0, -1, -1), 6, 6);
}

void IconIndexWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
}

void IconIndexWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
//    auto view = mDelegate->getView();
//    if (!view->selectionModel()->selectedIndexes().contains(mIndex)) {
//        this->close();
//        return;
//    }

//    mDelegate->getView()->activated(mIndex);
//    mDelegate->getView()->setIndexWidget(mIndex, nullptr);
    return;
}

void IconIndexWidget::updateItem()
{
    auto view = mDelegate->getView();
    mDelegate->initStyleOption(&mOption, mIndex);
    QSize size = mDelegate->sizeHint(mOption, mIndex);
    auto visualRect = mDelegate->getView()->visualRect(mIndex);
    auto rectCopy = visualRect;
    move(visualRect.topLeft());
    setFixedWidth(visualRect.width());

    mOption.rect.setWidth(visualRect.width());

    int rawHeight = size.height();
    auto textSize = graceful::IconViewTextHelper::getTextSizeForIndex(mOption, mIndex, 2);
    int fixedHeight = 5 + mDelegate->getView()->iconSize().height() + 5 + textSize.height() + 10;

    int y_bottom = rectCopy.y() + fixedHeight;
    qDebug() << "Y:" <<rectCopy.y() <<fixedHeight <<mDelegate->getView()->height();
    mElideText = false;
    if ( y_bottom > mDelegate->getView()->height() && mOption.text.length() > ELIDE_TEXT_LENGTH) {
        mElideText = true;
        int  charWidth = mOption.fontMetrics.averageCharWidth();
        mOption.text = mOption.fontMetrics.elidedText(mOption.text, Qt::ElideRight, ELIDE_TEXT_LENGTH * charWidth);
        fixedHeight = 5 + mDelegate->getView()->iconSize().height() + 5 + textSize.height() + 10;
    }

    if (fixedHeight < rawHeight)
        fixedHeight = rawHeight;

    mOption.text = mIndex.data().toString();
    mOption.features |= QStyleOptionViewItem::WrapText;
    mOption.textElideMode = Qt::ElideNone;

    mOption.rect.moveTo(0, 0);

    auto rawTextRect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemText, &mOption, mOption.widget);
    auto iconSizeExcepted = mDelegate->getView()->iconSize();

    auto iconRect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &mOption, mOption.widget);

    auto y_delta = iconSizeExcepted.height() - iconRect.height();

    rawTextRect.setTop(iconRect.bottom() + y_delta + 5);
    rawTextRect.setHeight(9999);

    QFontMetrics fm(mCurrentFont);
    auto textRect = QApplication::style()->itemTextRect(fm, rawTextRect, Qt::AlignTop|Qt::AlignHCenter|Qt::TextWrapAnywhere, true, mOption.text);

    mTextRect = textRect;

    setFixedHeight(fixedHeight);
}

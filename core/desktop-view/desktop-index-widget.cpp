#include "desktop-index-widget.h"

#include "desktop-view.h"
#include "desktop-view-delegate.h"
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

DesktopIndexWidget::DesktopIndexWidget(DesktopViewDelegate *delegate, const QStyleOptionViewItem &option, const QModelIndex &index, QWidget *parent) : QWidget(parent)
{
    setContentsMargins(0, 0, 0, 0);
    m_option = option;

    m_index = index;
    m_delegate = delegate;

    m_current_font = QApplication::font();

    updateItem();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    connect(qApp, &QApplication::fontChanged, this, [=]() {
        m_delegate->getView()->setIndexWidget(m_index, nullptr);
    });
#endif

    auto view = m_delegate->getView();
}

DesktopIndexWidget::~DesktopIndexWidget()
{

}

void DesktopIndexWidget::paintEvent(QPaintEvent *e)
{
    auto view = m_delegate->getView();
    auto visualRect = m_delegate->getView()->visualRect(m_index);
    if (this->pos() != visualRect.topLeft()) {
        move(visualRect.topLeft());
        return;
    }

    if (!view->selectionModel()->selectedIndexes().contains(m_index)) {
        this->close();
        return;
    }

    Q_UNUSED(e)
    QPainter p(this);
    auto bgColor = m_option.palette.highlight().color();
    p.save();
    p.setPen(Qt::transparent);
    bgColor.setAlpha(255*0.7);
    p.setBrush(bgColor);
    p.drawRoundedRect(this->rect(), 6, 6);
    p.restore();

    auto opt = m_option;
    auto iconSizeExcepted = m_delegate->getView()->iconSize();
    auto iconRect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, opt.widget);
    int y_delta = iconSizeExcepted.height() - iconRect.height();
    opt.rect.moveTo(opt.rect.x(), opt.rect.y() + y_delta);

    int maxTextHight = this->height() - iconSizeExcepted.height() - 10;

    // draw icon
    opt.text = nullptr;
    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, &p, m_delegate->getView());

    p.save();
    p.translate(0, 5 + m_delegate->getView()->iconSize().height() + 5);

    if (b_elide_text)
    {
        int  charWidth = opt.fontMetrics.averageCharWidth();
        m_option.text = opt.fontMetrics.elidedText(m_option.text, Qt::ElideRight, ELIDE_TEXT_LENGTH * charWidth);
    }
    p.save();

    auto expectedSize = IconViewTextHelper::getTextSizeForIndex(m_option, m_index, 2);
    QPixmap pixmap(expectedSize);
    pixmap.fill(Qt::transparent);
    QPainter shadowPainter(&pixmap);
    shadowPainter.setPen(Qt::black);
    IconViewTextHelper::paintText(&shadowPainter, m_option, m_index, maxTextHight, 2, 0, false, Qt::black);
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
    p.setPen(m_option.palette.highlightedText().color());
    IconViewTextHelper::paintText(&p, m_option, m_index, 9999, 2, 0, false);
    p.restore();

    bgColor.setAlpha(255*0.8);
    p.setPen(bgColor);
    p.drawRoundedRect(this->rect().adjusted(0, 0, -1, -1), 6, 6);
}

void DesktopIndexWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
}

void DesktopIndexWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    auto view = m_delegate->getView();
    if (!view->selectionModel()->selectedIndexes().contains(m_index)) {
        this->close();
        return;
    }

    m_delegate->getView()->activated(m_index);
    m_delegate->getView()->setIndexWidget(m_index, nullptr);
    return;
}

void DesktopIndexWidget::updateItem()
{
    auto view = m_delegate->getView();
    m_delegate->initStyleOption(&m_option, m_index);
    QSize size = m_delegate->sizeHint(m_option, m_index);
    auto visualRect = m_delegate->getView()->visualRect(m_index);
    auto rectCopy = visualRect;
    move(visualRect.topLeft());
    setFixedWidth(visualRect.width());

    m_option.rect.setWidth(visualRect.width());

    int rawHeight = size.height();
    auto textSize = graceful::IconViewTextHelper::getTextSizeForIndex(m_option, m_index, 2);
    int fixedHeight = 5 + m_delegate->getView()->iconSize().height() + 5 + textSize.height() + 10;

    int y_bottom = rectCopy.y() + fixedHeight;
    qDebug() << "Y:" <<rectCopy.y() <<fixedHeight <<m_delegate->getView()->height();
    b_elide_text = false;
    if ( y_bottom > m_delegate->getView()->height() && m_option.text.length() > ELIDE_TEXT_LENGTH) {
        b_elide_text = true;
        int  charWidth = m_option.fontMetrics.averageCharWidth();
        m_option.text = m_option.fontMetrics.elidedText(m_option.text, Qt::ElideRight, ELIDE_TEXT_LENGTH * charWidth);
        fixedHeight = 5 + m_delegate->getView()->iconSize().height() + 5 + textSize.height() + 10;
    }

    if (fixedHeight < rawHeight)
        fixedHeight = rawHeight;

    m_option.text = m_index.data().toString();
    m_option.features |= QStyleOptionViewItem::WrapText;
    m_option.textElideMode = Qt::ElideNone;

    m_option.rect.moveTo(0, 0);

    auto rawTextRect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemText, &m_option, m_option.widget);
    auto iconSizeExcepted = m_delegate->getView()->iconSize();

    auto iconRect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &m_option, m_option.widget);

    auto y_delta = iconSizeExcepted.height() - iconRect.height();

    rawTextRect.setTop(iconRect.bottom() + y_delta + 5);
    rawTextRect.setHeight(9999);

    QFontMetrics fm(m_current_font);
    auto textRect = QApplication::style()->itemTextRect(fm, rawTextRect, Qt::AlignTop|Qt::AlignHCenter|Qt::TextWrapAnywhere, true, m_option.text);

    m_text_rect = textRect;

    setFixedHeight(fixedHeight);
}

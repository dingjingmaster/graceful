#include "icon-view-delegate.h"

#include "file/file.h"
#include "icon-view.h"
#include "file-model/file-model.h"
#include "utils/icon-view-editor.h"
#include "utils/icon-view-text-helper.h"

#include <QWidget>
#include <QPainter>
#include <QFileInfo>
#include <QPushButton>
#include <QApplication>
#include <qstyleoption.h>
#include <QStyledItemDelegate>
#include <qitemeditorfactory.h>
#include <private/qitemeditorfactory_p.h>
#include <private/qabstractitemdelegate_p.h>


#include <QDebug>
#include <QTimer>

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

using namespace graceful;

IconViewDelegate::IconViewDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    mStyledButton = new QPushButton;
}

IconViewDelegate::~IconViewDelegate()
{
    mStyledButton->deleteLater();
}

void IconViewDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const
{
    QVariant value = index.data(Qt::FontRole);
    if (value.isValid() && !value.isNull()) {
        option->font = qvariant_cast<QFont>(value).resolve(option->font);
        option->fontMetrics = QFontMetrics(option->font);
    }

    value = index.data(Qt::TextAlignmentRole);
    if (value.isValid() && !value.isNull())
        option->displayAlignment = Qt::Alignment(value.toInt());

    option->index = index;
    value = index.data(Qt::CheckStateRole);
    if (value.isValid() && !value.isNull()) {
        option->features |= QStyleOptionViewItem::HasCheckIndicator;
        option->checkState = static_cast<Qt::CheckState>(value.toInt());
    }

    value = index.data(Qt::DisplayRole);
    if (value.isValid() && !value.isNull()) {
        option->features |= QStyleOptionViewItem::HasDisplay;
        option->text = displayText(value, option->locale);
    }

    option->backgroundBrush = qvariant_cast<QBrush>(index.data(Qt::BackgroundRole));
    option->styleObject = nullptr;
}

void IconViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    QStyle* style = QApplication::style();

    painter->save();
    auto view = getView();

    if (view->state() == IconView::DraggingState) {
        if (auto widget = view->indexWidget(index)) {
            view->setIndexWidget(index, nullptr);
        }

        if (view->selectionModel()->selection().contains(index)) {
            painter->setOpacity(0.8);
        }
    } else if (opt.state.testFlag(QStyle::State_Selected)) {
        if (view->indexWidget(index)) {
            opt.text = nullptr;
        }
    }

    // paint background
    if (!view->indexWidget(index)) {
        painter->save();
        painter->setClipRect(opt.rect);
        painter->setRenderHint(QPainter::Antialiasing);
        if (opt.state.testFlag(QStyle::State_MouseOver) && !opt.state.testFlag(QStyle::State_Selected)) {
            QColor color = mStyledButton->palette().highlight().color();
            color.setAlpha(255 * 0.3);
            color.setAlpha(255 * 0.5);
            painter->setPen(color.darker(100));
            painter->setBrush(color);
            painter->drawRoundedRect(opt.rect.adjusted(1, 1, -1, -1), 6, 6);
        }
        if (opt.state.testFlag(QStyle::State_Selected)) {
            QColor color = mStyledButton->palette().highlight().color();
            color.setAlpha(255 * 0.7);
            color.setAlpha(255 * 0.8);
            painter->setPen(color);
            painter->setBrush(color);
            painter->drawRoundedRect(opt.rect.adjusted(1, 1, -1, -1), 6, 6);
        }
        painter->restore();
    }

    auto iconSizeExpected = view->iconSize();
    auto iconRect = style->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, opt.widget);
    int y_delta = iconSizeExpected.height() - iconRect.height();
    opt.rect.translate(0, y_delta);

    int maxTextHight = opt.rect.height() - iconSizeExpected.height() - 10;
    if (maxTextHight < 0) {
        maxTextHight = 0;
    }

    // paint icon item
    auto color = QColor(230, 230, 230);
    opt.palette.setColor(QPalette::Text, color);
    color.setRgb(240, 240, 240);
    opt.palette.setColor(QPalette::HighlightedText, color);

    auto text = opt.text;
    opt.text = nullptr;

    painter->save();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
    painter->restore();

    opt.text = text;

    painter->save();
    painter->translate(opt.rect.topLeft());
    painter->translate(0, -y_delta);

    // paint text shadow
    painter->save();
    painter->translate(1, 1 + iconSizeExpected.height() + 10);

    auto expectedSize = IconViewTextHelper::getTextSizeForIndex(opt, index, 2);
    QPixmap pixmap(expectedSize);
    pixmap.fill(Qt::transparent);
    QPainter shadowPainter(&pixmap);
    QColor shadow = Qt::black;
    shadowPainter.setPen(shadow);
    IconViewTextHelper::paintText(&shadowPainter, opt, index, maxTextHight, 0, 2, false, shadow);
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

    painter->translate(-2, -2);
    painter->drawImage(0, 0, shadowImage);

    painter->restore();

    // paint text
    painter->save();
    painter->translate(0, 0 + iconSizeExpected.height() + 10);
    QColor textColor = Qt::white;
    textColor.setAlphaF(0.9);
    painter->setPen(textColor);
    IconViewTextHelper::paintText(painter, opt, index, maxTextHight, 0, 2, false);
    painter->restore();

    painter->restore();

    // paint link icon and locker icon
    File file(index.data(FileModel::FileUriRole).toString());
    if ((index.data(FileModel::FileUriRole).toString() != "computer:///") && (index.data(Qt::UserRole).toString() != "trash:///")) {
        QSize lockerIconSize = QSize(16, 16);
        int offset = 8;
        switch (view->zoomLevel()) {
        case IconView::Small: {
            lockerIconSize = QSize(8, 8);
            offset = 10;
            break;
        }
        case IconView::Normal: {
            break;
        }
        case IconView::Large: {
            offset = 4;
            lockerIconSize = QSize(24, 24);
            break;
        }
        case IconView::Huge: {
            offset = 2;
            lockerIconSize = QSize(32, 32);
            break;
        }
        default: {
            break;
        }
        }
        auto topRight = opt.rect.topRight();
        topRight.setX(topRight.x() - opt.rect.width() + 10);
        topRight.setY(topRight.y() + 10);
        auto linkRect = QRect(topRight, lockerIconSize);
        if (!file.canRead()) {
            QIcon symbolicLinkIcon = QIcon::fromTheme("emblem-unreadable");
            symbolicLinkIcon.paint(painter, linkRect, Qt::AlignCenter);
        } else if(!file.canWrite() && !file.canExecute()) {
            QIcon symbolicLinkIcon = QIcon::fromTheme("emblem-readonly");
            symbolicLinkIcon.paint(painter, linkRect, Qt::AlignCenter);
        }
    }

    if (index.data(FileModel::FileUriRole + 1).toBool()) {
        QSize symbolicIconSize = QSize(16, 16);
        int offset = 8;
        switch (view->zoomLevel()) {
        case IconView::Small: {
            symbolicIconSize = QSize(8, 8);
            offset = 10;
            break;
        }
        case IconView::Normal: {
            break;
        }
        case IconView::Large: {
            offset = 4;
            symbolicIconSize = QSize(24, 24);
            break;
        }
        case IconView::Huge: {
            offset = 2;
            symbolicIconSize = QSize(32, 32);
            break;
        }
        default: {
            break;
        }
        }
        auto topRight = opt.rect.topRight();
        topRight.setX(topRight.x() - offset - symbolicIconSize.width());
        topRight.setY(topRight.y() + offset);
        auto linkRect = QRect(topRight, symbolicIconSize);
        QIcon symbolicLinkIcon = QIcon::fromTheme("emblem-symbolic-link");
        symbolicLinkIcon.paint(painter, linkRect, Qt::AlignCenter);
    }

    painter->restore();
}

QSize IconViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto view = qobject_cast<IconView*>(parent());
    auto zoomLevel = view->zoomLevel();
    switch (zoomLevel) {
    case IconView::Small:
        return QSize(60, 70);
    case IconView::Normal:
        return QSize(90, 100);
    case IconView::Large:
        return QSize(105, 128);
    case IconView::Huge:
        return QSize(120, 150);
    default:
        return QSize(90, 100);
    }
}

QWidget *IconViewDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto edit = new IconViewEditor(parent);
    auto font = option.font;
    auto view = qobject_cast<IconView*>(this->parent());
    switch (view->zoomLevel()) {
    case IconView::Small:
        font.setPixelSize(int(font.pixelSize() * 0.8));
        break;
    case IconView::Large:
        font.setPixelSize(int(font.pixelSize() * 1.2));
        break;
    case IconView::Huge:
        font.setPixelSize(int(font.pixelSize() * 1.4));
        break;
    default:
        break;
    }

    edit->setFont(font);

    edit->setContentsMargins(0, 0, 0, 0);
    edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    edit->setMinimumSize(sizeHint(option, index).width(), 54);

    edit->setText(index.data(Qt::DisplayRole).toString());
    edit->setAlignment(Qt::AlignCenter);
    QTimer::singleShot(1, edit, [=]() {
        edit->minimalAdjust();
    });

    getView()->setEditFlag(true);
    connect(edit, &IconViewEditor::returnPressed, getView(), [=]() {
        this->setModelData(edit, nullptr, index);
        edit->deleteLater();
        getView()->setEditFlag(false);
    });
    connect(edit, &IconViewEditor::destroyed, getView(), [=](){
        getView()->setEditFlag(false);
    });

    return edit;
}

void IconViewDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    IconViewEditor* edit = qobject_cast<IconViewEditor*>(editor);
    if (!edit) {
        return;
    }

    auto cursor = edit->textCursor();
    cursor.setPosition(0, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    if (edit->toPlainText().contains(".") && !edit->toPlainText().startsWith(".")) {
        cursor.movePosition(QTextCursor::WordLeft, QTextCursor::KeepAnchor, 1);
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);
    }
    edit->setTextCursor(cursor);

    Q_UNUSED(index)
}

void IconViewDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    IconViewEditor* edit = qobject_cast<IconViewEditor*>(editor);
    if (!edit) {
        return;
    }

    auto newName = edit->toPlainText();
    auto oldName = index.data(Qt::DisplayRole).toString();
    if (newName.isNull()) {
        return;
    }

    if (newName == "." || newName == ".." || newName.trimmed() == "" || newName.contains("\\")) {
        newName = "";
    }

    if (newName.length() > 0 && newName != oldName) {
        // FIXME:// rename
    } else if (newName == oldName) {
        getView()->selectionModel()->select(index, QItemSelectionModel::Select);
        getView()->setFocus();
    }
}

IconView* IconViewDelegate::getView() const
{
    auto view = qobject_cast<IconView*>(parent());
    return view;
}


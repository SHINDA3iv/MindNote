#include "workspace.h"
#include "checkbox_item.h"
#include "file_item.h"
#include "image_item.h"
#include "list_item.h"
#include "nested_workspace_item.h"
#include "text_item.h"
#include "title_item.h"

#include <QScrollArea>
#include <QMenu>

Workspace::Workspace(const QString &name, QWidget *parent) : QWidget(parent), _workspaceName(name)
{
    QVBoxLayout *contentLayout = new QVBoxLayout(this);

    _titleLabel = new QLabel(_workspaceName, this);
    _titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = _titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    _titleLabel->setFont(titleFont);
    contentLayout->addWidget(_titleLabel);

    _contentWidget = new QWidget(this);
    _layout = new QVBoxLayout(_contentWidget);
    _contentWidget->setLayout(_layout);

    _scrollArea = new QScrollArea(this);
    _scrollArea->setWidget(_contentWidget);
    // _scrollArea->setWidgetResizable(true);
    _scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    _scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    _scrollArea->setWidgetResizable(false);
    _contentWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    contentLayout->addWidget(_scrollArea);
    setLayout(contentLayout);
}

QString Workspace::getName() const
{
    return _workspaceName;
}

void Workspace::setName(const QString &name)
{
    _workspaceName = name;
    if (_titleLabel) {
        _titleLabel->setText(name);
    }
}

void Workspace::addItem(AbstractWorkspaceItem *item)
{
    _items.append(item);

    if (_spacerItem) {
        _layout->removeItem(_spacerItem);
        delete _spacerItem;
        _spacerItem = nullptr;
    }

    // if (auto resizableItem = static_cast<ResizableItem *>(item))
    //     connect(resizableItem, &ResizableItem::resized, this, &Workspace::adjustLayout);

    item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    item->setMinimumSize(50, 50);

    item->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(item, &QWidget::customContextMenuRequested, this, [this, item](const QPoint &pos) {
        QMenu contextMenu(this);

        QAction *deleteAction = contextMenu.addAction("Удалить элемент");
        connect(deleteAction, &QAction::triggered, [item, this]() {
            removeItem(item);
        });

        contextMenu.exec(item->mapToGlobal(pos));
    });

    _layout->addWidget(item);

    _spacerItem = new QSpacerItem(20, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    _layout->addItem(_spacerItem);
    // adjustLayout();
    updateContentSize();
}

void Workspace::removeItem(AbstractWorkspaceItem *item)
{
    _items.removeOne(item);
    _layout->removeWidget(item);
    delete item;
    adjustLayout();
}

QJsonObject Workspace::serialize() const
{
    QJsonObject json;
    json["name"] = _workspaceName;

    QJsonArray itemArray;
    for (const AbstractWorkspaceItem *item : _items) {
        itemArray.append(item->serialize());
    }
    json["items"] = itemArray;

    return json;
}

void Workspace::deserialize(const QJsonObject &json)
{
    if (json.contains("name")) {
        setName(json["name"].toString());
    }

    if (json.contains("items")) {
        QJsonArray itemArray = json["items"].toArray();
        for (const QJsonValue &itemVal : itemArray) {
            QJsonObject itemObj = itemVal.toObject();
            QString type = itemObj["type"].toString();
            AbstractWorkspaceItem *item = nullptr;

            if (type == "TitleItem") {
                item = new TitleItem();
            } else if (type == "TextItem") {
                item = new TextItem();
            } else if (type == "ListItem") {
                item = new ListItem();
            } else if (type == "CheckboxItem") {
                item = new CheckboxItem();
            } else if (type == "ImageItem") {
                item = new ImageItem();
            } else if (type == "FileItem") {
                item = new FileItem();
            } else if (type == "NestedWorkspaceItem") {
                item = new NestedWorkspaceItem();
            }

            if (item) {
                item->deserialize(itemObj);
                addItem(item);
            }
        }
    }
}

void Workspace::adjustLayout()
{
    int currentY = 0;
    for (AbstractWorkspaceItem *item : _items) {
        QRect itemGeometry = item->geometry();
        itemGeometry.moveTop(currentY);
        item->setGeometry(itemGeometry);

        currentY += itemGeometry.height() + _layout->spacing();
    }

    // if (_spacerItem) {
    //     _spacerItem->changeSize(20, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    // }

    // qDebug() << "Spacer " << _spacerItem->geometry().height() << " ЭлГеом1 "
    //          << currentY + _layout->spacing() - _spacerItem->geometry().height() << " ЭлГеом "
    //          << currentY + _layout->spacing() << " парент " << _layout->parentWidget()->height()
    //          << parentWidget()->sizeHint() << parentWidget()->sizePolicy() << " ЭлХинт "
    //          << totalHeight;

    // updateContentSize();

    // _scrollArea->updateGeometry();
    // _scrollArea->update();
}

void Workspace::updateContentSize()
{
    int totalHeight = 0;
    for (AbstractWorkspaceItem *item : _items) {
        totalHeight += item->geometry().height() + _layout->spacing();
    }

    if (_spacerItem) {
        totalHeight += _spacerItem->geometry().height();
    }

    int totalWidth = _scrollArea->viewport()->width();

    // Обновляем размер `_contentWidget`
    totalHeight = _contentWidget->height() > totalHeight ? _contentWidget->height() : totalHeight;
    _contentWidget->resize(totalWidth, totalHeight);

    // Обновляем scrollArea
    _scrollArea->updateGeometry();
    _scrollArea->update();
}

// Метод getItems в классе Workspace возвращает текущие элементы
QList<AbstractWorkspaceItem *> Workspace::getItems() const
{
    return _items;
}

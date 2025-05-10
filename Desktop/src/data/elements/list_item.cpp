#include "list_item.h"

#include <QInputDialog>

ListItem::ListItem(ListType type, Workspace *parent) :
    ResizableItem(parent),
    _listWidget(new QListWidget(this)),
    _listType(type)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(_listWidget);
    setLayout(layout);

    resize(width(), 250);

    setupContextMenu();
    
    _listWidget->installEventFilter(this);
}

QString ListItem::type() const
{
    return _listType == Ordered ? "OrderedListItem" : "UnorderedListItem";
}

void ListItem::addItemToList(const QString &text)
{
    if (_listType == Ordered) {
        _listWidget->addItem(QString::number(_listWidget->count() + 1) + ". " + text);
    } else {
        _listWidget->addItem(text);
    }
}

QJsonObject ListItem::serialize() const
{
    QJsonObject json;
    json["type"] = type();
    QJsonArray items;
    for (int i = 0; i < _listWidget->count(); ++i) {
        items.append(_listWidget->item(i)->text());
    }
    json["items"] = items;
    return json;
}

void ListItem::deserialize(const QJsonObject &json)
{
    if (json.contains("items")) {
        QJsonArray items = json["items"].toArray();
        _listWidget->clear();
        for (const auto &item : items) {
            _listWidget->addItem(item.toString());
        }
    }
}

void ListItem::removeItemFromList(int index)
{
    if (index >= 0 && index < _listWidget->count()) {
        delete _listWidget->takeItem(index);

        if (_listType == Ordered) {
            for (int i = 0; i < _listWidget->count(); ++i) {
                QString itemText = _listWidget->item(i)->text();
                int dotIndex = itemText.indexOf(". ");
                if (dotIndex != -1) {
                    itemText = itemText.mid(dotIndex + 2);
                }
                _listWidget->item(i)->setText(QString::number(i + 1) + ". " + itemText);
            }
        }
    }
}

void ListItem::editItemInList(int index)
{
    if (index >= 0 && index < _listWidget->count()) {
        bool ok;
        QString currentText = _listWidget->item(index)->text();
        QString text = currentText;
        if (_listType == Ordered) {
            int dotIndex = currentText.indexOf(". ");
            if (dotIndex != -1) {
                text = currentText.mid(dotIndex + 2);
            }
        }
        QString newText = QInputDialog::getText(
         this, "Редактировать элемент", "Введите новый текст:", QLineEdit::Normal, text, &ok);
        if (ok && !newText.isEmpty()) {
            if (_listType == Ordered) {
                _listWidget->item(index)->setText(QString::number(index + 1) + ". " + newText);
            } else {
                _listWidget->item(index)->setText(newText);
            }
        }
    }
}

void ListItem::addCustomContextMenuActions(QMenu *contextMenu)
{
    QAction *addAction = contextMenu->addAction("Добавить элемент");
    connect(addAction, &QAction::triggered, this, &ListItem::addNewItem);

    QAction *removeAction = contextMenu->addAction("Удалить элемент");
    connect(removeAction, &QAction::triggered, this, &ListItem::removeSelectedItem);

    QAction *editAction = contextMenu->addAction("Редактировать элемент");
    connect(editAction, &QAction::triggered, this, &ListItem::editSelectedItem);
}

void ListItem::addNewItem()
{
    bool ok;
    QString text = QInputDialog::getText(this, "Добавить элемент",
                                         "Введите текст элемента:", QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty()) {
        addItemToList(text);
    }
}

void ListItem::removeSelectedItem()
{
    int index = _listWidget->currentRow();
    if (index != -1) {
        removeItemFromList(index);
    }
}

void ListItem::editSelectedItem()
{
    int index = _listWidget->currentRow();
    if (index != -1) {
        editItemInList(index);
    }
}

void ListItem::mousePressEvent(QMouseEvent *event)
{
    QListWidgetItem *clickedItem = _listWidget->itemAt(event->pos());
    if (!clickedItem) {
        _listWidget->clearSelection();
        _listWidget->clearFocus();
    }

    ResizableItem::mousePressEvent(event);
}

void ListItem::createContextMenu(const QPoint &pos)
{
    QMenu contextMenu(this);

    if (_clickedIndex != -1) {
        QAction *removeAction = contextMenu.addAction("Удалить элемент");
        connect(removeAction, &QAction::triggered, this, &ListItem::removeSelectedItem);

        QAction *editAction = contextMenu.addAction("Редактировать элемент");
        connect(editAction, &QAction::triggered, this, &ListItem::editSelectedItem);
    } else {
        QAction *addAction = contextMenu.addAction("Добавить элемент");
        connect(addAction, &QAction::triggered, this, &ListItem::addNewItem);

        QAction *deleteAction = contextMenu.addAction("Удалить список");
        connect(deleteAction, &QAction::triggered, this, &AbstractWorkspaceItem::deleteItem);
    }

    contextMenu.exec(_listWidget->mapToGlobal(pos));
}

void ListItem::setupContextMenu()
{
    _listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_listWidget, &QListWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        QListWidgetItem *clickedItem = _listWidget->itemAt(pos);
        if (clickedItem) {
            _clickedIndex = _listWidget->row(clickedItem);
        } else {
            _clickedIndex = -1;
        }
        createContextMenu(pos);
    });
}

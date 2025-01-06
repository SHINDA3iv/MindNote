#include "list_item.h"

ListItem::ListItem(ListType type, Workspace *parent) :
    ResizableItem(parent),
    _listWidget(new QListWidget(this)),
    _listType(type)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(_listWidget);
    setLayout(layout);

    if (_listType == Ordered) {
        _listWidget->setStyleSheet("QListWidget::item { list-style-type: decimal; }");
    } else {
        _listWidget->setStyleSheet("QListWidget::item { list-style-type: disc; }");
    }
}

QString ListItem::type() const
{
    return _listType == Ordered ? "OrderedListItem" : "UnorderedListItem";
}

void ListItem::addItemToList(const QString &text)
{
    _listWidget->addItem(text);
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

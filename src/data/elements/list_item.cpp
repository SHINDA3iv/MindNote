#include "list_item.h"

ListItem::ListItem(ListType type, QWidget *parent) :
    AbstractWorkspaceItem(parent),
    listWidget(new QListWidget(this)),
    listType(type)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(listWidget);
    setLayout(layout);
}

QString ListItem::type() const
{
    return listType == Ordered ? "OrderedListItem" : "UnorderedListItem";
}

QJsonObject ListItem::serialize() const
{
    QJsonObject json;
    json["type"] = type();
    QJsonArray items;
    for (int i = 0; i < listWidget->count(); ++i) {
        items.append(listWidget->item(i)->text());
    }
    json["items"] = items;
    return json;
}

void ListItem::deserialize(const QJsonObject &json)
{
    if (json.contains("items")) {
        QJsonArray items = json["items"].toArray();
        listWidget->clear();
        for (const auto &item : items) {
            listWidget->addItem(item.toString());
        }
    }
}

#include "workspace.h"

Workspace::Workspace(const QString &name, QWidget *parent) : QWidget(parent), workspaceName(name)
{
    layout = new QVBoxLayout(this);
    setLayout(layout);
}

QString Workspace::getName() const
{
    return workspaceName;
}

void Workspace::setName(const QString &name)
{
    workspaceName = name;
}

void Workspace::addItem(AbstractWorkspaceItem *item)
{
    items.append(item);
    layout->addWidget(item);
}

void Workspace::removeItem(AbstractWorkspaceItem *item)
{
    items.removeOne(item);
    layout->removeWidget(item);
    delete item;
}

QJsonObject Workspace::serialize() const
{
    QJsonObject json;
    json["name"] = workspaceName;

    QJsonArray itemArray;
    for (const AbstractWorkspaceItem *item : items) {
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

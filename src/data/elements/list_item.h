#ifndef LISTITEM_H
#define LISTITEM_H

#include "abstract_workspace_item.h"

#include <QListWidget>
#include <QVBoxLayout>
#include <QJsonArray>

class ListItem : public AbstractWorkspaceItem
{
    Q_OBJECT

public:
    enum ListType
    {
        Ordered,
        Unordered
    };

    explicit ListItem(ListType type = Unordered, QWidget *parent = nullptr);

    QString type() const override;

    QJsonObject serialize() const override;

    void deserialize(const QJsonObject &json) override
    {
        if (json.contains("items")) {
            QJsonArray items = json["items"].toArray();
            listWidget->clear();
            for (const auto &item : items) {
                listWidget->addItem(item.toString());
            }
        }
    }

private:
    QListWidget *listWidget;
    ListType listType;
};

#endif // LISTITEM_H

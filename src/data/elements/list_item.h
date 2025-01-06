#ifndef LISTITEM_H
#define LISTITEM_H

#include "resizable_item.h"

#include <QListWidget>
#include <QVBoxLayout>
#include <QJsonArray>
#include <QPointer>

class ListItem : public ResizableItem
{
    Q_OBJECT

public:
    enum ListType
    {
        Ordered, // Нумерованный список
        Unordered // Ненумерованный список
    };

    explicit ListItem(ListType type = Unordered, Workspace *parent = nullptr);

    QString type() const override;
    void addItemToList(const QString &text);

    QJsonObject serialize() const override;
    void deserialize(const QJsonObject &json) override;

private:
    QPointer<QListWidget> _listWidget;
    ListType _listType;
};

#endif // LISTITEM_H

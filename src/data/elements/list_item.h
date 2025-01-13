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

    ListItem(ListType type, Workspace *parent = nullptr);

    QString type() const override;

    void addItemToList(const QString &text);
    void removeItemFromList(int index);
    void editItemInList(int index);

    QJsonObject serialize() const override;
    void deserialize(const QJsonObject &json) override;

protected:
    void addCustomContextMenuActions(QMenu *contextMenu) override;
    void mousePressEvent(QMouseEvent *event) override;
    void createContextMenu(const QPoint &pos) override;

protected slots:
    void addNewItem();
    void removeSelectedItem();
    void editSelectedItem();

private:
    QPointer<QListWidget> _listWidget;
    ListType _listType;

    int _clickedIndex = -1;
    void setupContextMenu();
};

#endif // LISTITEM_H

#ifndef WORKSPACEITEM_H
#define WORKSPACEITEM_H

#include <qmenu.h>
#include <QWidget>
#include <QString>
#include <QJsonObject>

class Workspace;

class AbstractWorkspaceItem : public QWidget
{
    Q_OBJECT
public:
    explicit AbstractWorkspaceItem(QWidget *parent = nullptr) : QWidget(parent)
    {
        setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this, &QWidget::customContextMenuRequested, this,
                &AbstractWorkspaceItem::createContextMenu);
    }
    virtual ~AbstractWorkspaceItem()
    {}

    // Возвращает тип элемента (для сериализации)
    virtual QString type() const = 0;
    // Сериализация элемента в JSON
    virtual QJsonObject serialize() const = 0;
    // Десериализация элемента из JSON
    virtual void deserialize(const QJsonObject &json) = 0;

protected slots:
    virtual void createContextMenu(const QPoint &pos)
    {
        QMenu contextMenu(this);

        addCustomContextMenuActions(&contextMenu);

        QAction *deleteAction = contextMenu.addAction("Удалить элемент");
        connect(deleteAction, &QAction::triggered, this, &AbstractWorkspaceItem::deleteItem);

        contextMenu.exec(mapToGlobal(pos));
    }

    virtual void addCustomContextMenuActions(QMenu *contextMenu) {};

public:
    virtual void deleteItem()
    {
        emit itemDeleted(this);
    }

signals:
    void itemDeleted(AbstractWorkspaceItem *item);
};

#endif // WORKSPACEITEM_H

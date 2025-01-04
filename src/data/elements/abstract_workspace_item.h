#ifndef WORKSPACEITEM_H
#define WORKSPACEITEM_H

#include <QWidget>
#include <QString>
#include <QJsonObject>

class AbstractWorkspaceItem : public QWidget
{
    Q_OBJECT
public:
    explicit AbstractWorkspaceItem(QWidget *parent = nullptr) : QWidget(parent)
    {}
    virtual ~AbstractWorkspaceItem()
    {}

    // Возвращает тип элемента (для сериализации)
    virtual QString type() const = 0;
    // Сериализация элемента в JSON
    virtual QJsonObject serialize() const = 0;
    // Десериализация элемента из JSON
    virtual void deserialize(const QJsonObject &json) = 0;
};

#endif // WORKSPACEITEM_H

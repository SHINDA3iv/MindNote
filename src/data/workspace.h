#ifndef WORKSPACE_H
#define WORKSPACE_H

#include "abstract_workspace_item.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QJsonObject>
#include <QList>
#include <QJsonArray>
#include <QPointer>
#include <QLabel>
#include <QScrollArea>

class Workspace : public QWidget
{
    Q_OBJECT

public:
    explicit Workspace(const QString &name = "Новое пространство", QWidget *parent = nullptr);

    QString getName() const;
    void setName(const QString &name);

    void addItem(AbstractWorkspaceItem *item);
    void removeItem(AbstractWorkspaceItem *item);

    QJsonObject serialize() const;
    void deserialize(const QJsonObject &json);

    QList<AbstractWorkspaceItem *> getItems() const;

    void addItemByType(const QString &type);

    void setIcon(const QIcon &icon);
    QLabel *getIcon();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateContentSize();

    QString _workspaceName;

    QPointer<QVBoxLayout> _layout;
    QPointer<QScrollArea> _scrollArea;
    QPointer<QWidget> _contentWidget;
    QPointer<QLabel> _titleLabel;
    QPointer<QLabel> _iconLabel;

    QList<AbstractWorkspaceItem *> _items;
    QSpacerItem *_spacerItem { nullptr };
};

#endif // WORKSPACE_H

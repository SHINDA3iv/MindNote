#ifndef NESTEDWORKSPACEITEM_H
#define NESTEDWORKSPACEITEM_H

#include "abstract_workspace_item.h"
#include "workspace.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QPointer>

class NestedWorkspaceItem : public AbstractWorkspaceItem
{
    Q_OBJECT

public:
    explicit NestedWorkspaceItem(const QString &workspaceName = "Вложенное пространство",
                                 Workspace *parent = nullptr);

    QString type() const override;

    QJsonObject serialize() const override;
    void deserialize(const QJsonObject &json) override;

signals:
    void requestOpenWorkspace(const QString &workspaceId);

private slots:
    void openWorkspace();

private:
    QPointer<QPushButton> _workspaceButton;
    QString _workspaceName;
    QString _workspaceId; // Уникальный идентификатор вложенного пространства
};

#endif // NESTEDWORKSPACEITEM_H

#include "nested_workspace_item.h"

NestedWorkspaceItem::NestedWorkspaceItem(const QString &workspaceName, QWidget *parent) :
    AbstractWorkspaceItem(parent),
    workspaceButton(new QPushButton(workspaceName, this)),
    workspaceName(workspaceName)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(workspaceButton);
    setLayout(layout);

    connect(workspaceButton, &QPushButton::clicked, this, &NestedWorkspaceItem::openWorkspace);
}

QString NestedWorkspaceItem::type() const
{
    return "NestedWorkspaceItem";
}

QJsonObject NestedWorkspaceItem::serialize() const
{
    QJsonObject json;
    json["type"] = type();
    json["workspaceName"] = workspaceName;
    json["workspaceId"] = workspaceId;
    return json;
}

void NestedWorkspaceItem::deserialize(const QJsonObject &json)
{
    if (json.contains("workspaceName")) {
        workspaceName = json["workspaceName"].toString();
        workspaceButton->setText(workspaceName);
    }
    if (json.contains("workspaceId")) {
        workspaceId = json["workspaceId"].toString();
    }
}

void NestedWorkspaceItem::openWorkspace()
{
    emit requestOpenWorkspace(workspaceId);
}

#include "nested_workspace_item.h"

NestedWorkspaceItem::NestedWorkspaceItem(const QString &workspaceName, Workspace *parent) :
    AbstractWorkspaceItem(parent),
    _workspaceButton(new QPushButton(workspaceName, this)),
    _workspaceName(workspaceName)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(_workspaceButton);
    setLayout(layout);

    connect(_workspaceButton, &QPushButton::clicked, this, &NestedWorkspaceItem::openWorkspace);
}

QString NestedWorkspaceItem::type() const
{
    return "NestedWorkspaceItem";
}

QJsonObject NestedWorkspaceItem::serialize() const
{
    QJsonObject json;
    json["type"] = type();
    json["workspaceName"] = _workspaceName;
    json["workspaceId"] = _workspaceId;
    return json;
}

void NestedWorkspaceItem::deserialize(const QJsonObject &json)
{
    if (json.contains("workspaceName")) {
        _workspaceName = json["workspaceName"].toString();
        _workspaceButton->setText(_workspaceName);
    }
    if (json.contains("workspaceId")) {
        _workspaceId = json["workspaceId"].toString();
    }
}

void NestedWorkspaceItem::openWorkspace()
{
    emit requestOpenWorkspace(_workspaceId);
}

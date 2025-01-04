#include "workspace_controller.h"

WorkspaceController::WorkspaceController(QObject *parent) : QObject(parent)
{}

Workspace *WorkspaceController::createWorkspace(const QString &name)
{
    Workspace *workspace = new Workspace(name);
    workspaces.append(workspace);
    return workspace;
}

void WorkspaceController::removeWorkspace(Workspace *workspace)
{
    workspaces.removeOne(workspace);
    delete workspace;
}

Workspace *WorkspaceController::getWorkspace(int index) const
{
    return workspaces.value(index, nullptr);
}

QList<Workspace *> WorkspaceController::getAllWorkspaces() const
{
    return workspaces;
}

QJsonObject WorkspaceController::serialize() const
{
    QJsonObject json;
    QJsonArray workspacesArray;

    for (const Workspace *workspace : workspaces) {
        workspacesArray.append(workspace->serialize());
    }

    json["workspaces"] = workspacesArray;
    return json;
}

void WorkspaceController::deserialize(const QJsonObject &json)
{
    if (json.contains("workspaces")) {
        QJsonArray workspacesArray = json["workspaces"].toArray();
        for (const QJsonValue &workspaceVal : workspacesArray) {
            QJsonObject workspaceObj = workspaceVal.toObject();
            QString name = workspaceObj["name"].toString();

            Workspace *workspace = createWorkspace(name);
            workspace->deserialize(workspaceObj);
        }
    }
}

void WorkspaceController::saveToFile(const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonObject json = serialize();
        file.write(QJsonDocument(json).toJson());
        file.close();
    }
}

void WorkspaceController::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject json = doc.object();
        deserialize(json);
        file.close();
    }
}

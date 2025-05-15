#include "workspace_controller.h"
#include <QUuid>

WorkspaceController::WorkspaceController(std::shared_ptr<LocalStorage> localStorage,
                                         QObject *parent) :
    QObject(parent),
    _localStorage(localStorage)
{
    loadWorkspaces();
}

Workspace *WorkspaceController::createWorkspace(const QString &name, const QString &id)
{
    QString workspaceId = id.isEmpty() ? QUuid::createUuid().toString() : id;
    Workspace *workspace = new Workspace(name);
    workspace->setProperty("id", workspaceId);
    
    // Set default workspace icon
    workspace->setIcon(QIcon(":/icons/workspace.png"));

    _workspaces.append(workspace);

    // Сохраняем изменения
    saveWorkspaces();

    return workspace;
}

void WorkspaceController::removeWorkspace(Workspace *workspace)
{
    if (!workspace)
        return;

    QString workspaceId = workspace->property("id").toString();
    _workspaces.removeOne(workspace);
    delete workspace;

    // Удаляем связанные элементы
    _localStorage->saveWorkspaceItems(workspaceId, QJsonArray());

    // Сохраняем изменения
    saveWorkspaces();
}

Workspace *WorkspaceController::getWorkspace(int index) const
{
    return _workspaces.value(index, nullptr);
}

QList<Workspace *> WorkspaceController::getAllWorkspaces() const
{
    return _workspaces;
}

Workspace *WorkspaceController::getWorkspaceById(const QString &id) const
{
    for (Workspace *workspace : _workspaces) {
        if (workspace->property("id").toString() == id) {
            return workspace;
        }
    }
    return nullptr;
}

QJsonObject WorkspaceController::serialize() const
{
    QJsonObject json;
    QJsonArray workspacesArray;

    for (const Workspace *workspace : _workspaces) {
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

void WorkspaceController::saveWorkspaces()
{
    QJsonArray workspacesArray;

    for (Workspace *workspace : _workspaces) {
        QJsonObject workspaceObj = workspace->serialize();
        workspaceObj["id"] = workspace->property("id").toString();
        workspacesArray.append(workspaceObj);

        // Сохраняем элементы рабочего пространства
        QJsonArray itemsArray;
        for (AbstractWorkspaceItem *item : workspace->getItems()) {
            itemsArray.append(item->serialize());
        }
        _localStorage->saveWorkspaceItems(workspace->property("id").toString(), itemsArray);
    }

    _localStorage->saveWorkspaces(workspacesArray);
}

void WorkspaceController::loadWorkspaces()
{
    QJsonArray workspacesArray = _localStorage->loadWorkspaces();

    for (const QJsonValue &workspaceVal : workspacesArray) {
        QJsonObject workspaceObj = workspaceVal.toObject();
        QString name = workspaceObj["name"].toString();
        QString id = workspaceObj["id"].toString();

        Workspace *workspace = createWorkspace(name, id);
        workspace->deserialize(workspaceObj);

        // Загружаем элементы рабочего пространства
        QJsonArray itemsArray = _localStorage->loadWorkspaceItems(id);
        workspace->deserializeItems(itemsArray);
    }
}

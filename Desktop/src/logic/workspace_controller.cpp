#include "workspace_controller.h"
#include <QUuid>
#include "../data/elements/SubspaceLinkItem.h"

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

Workspace *
WorkspaceController::createSubWorkspace(Workspace *parent, const QString &name, const QString &id)
{
    if (!parent || parent->hasSubWorkspaceWithName(name))
        return nullptr;
    QString subId = id.isEmpty() ? QUuid::createUuid().toString() : id;
    Workspace *sub = new Workspace(name);
    sub->setId(subId);
    parent->addSubWorkspace(sub);
    _workspaces.append(sub);
    // Добавляем ссылку-элемент в список элементов родителя
    auto *linkItem = new SubspaceLinkItem(sub, parent);
    QObject::connect(linkItem, &SubspaceLinkItem::subspaceLinkClicked, parent,
                     &Workspace::subWorkspaceClicked);
    parent->addItem(linkItem);
    saveWorkspaces();
    return sub;
}

QList<Workspace *> WorkspaceController::getRootWorkspaces() const
{
    QList<Workspace *> roots;
    for (Workspace *ws : _workspaces) {
        if (!ws->getParentWorkspace())
            roots.append(ws);
    }
    return roots;
}

Workspace *WorkspaceController::findWorkspaceById(const QString &id) const
{
    for (Workspace *ws : _workspaces) {
        if (ws->getId() == id)
            return ws;
    }
    return nullptr;
}

QJsonObject WorkspaceController::serialize() const
{
    QJsonObject json;
    QJsonArray workspacesArray;
    for (const Workspace *ws : _workspaces) {
        if (!ws->getParentWorkspace())
            workspacesArray.append(ws->serialize());
    }
    json["workspaces"] = workspacesArray;
    return json;
}

void WorkspaceController::deserialize(const QJsonObject &json)
{
    qDeleteAll(_workspaces);
    _workspaces.clear();
    QMap<QString, Workspace *> idMap;
    QList<QPair<SubspaceLinkItem *, QString>> linkItemsToFix;
    if (json.contains("workspaces")) {
        QJsonArray arr = json["workspaces"].toArray();
        std::function<Workspace *(const QJsonObject &, Workspace *)> loadTree =
         [&](const QJsonObject &obj, Workspace *parent) -> Workspace * {
            QString name = obj["name"].toString();
            QString id = obj["id"].toString();
            Workspace *ws = new Workspace(name);
            ws->setId(id);
            ws->setParentWorkspace(parent);
            ws->deserialize(obj);
            idMap[id] = ws;
            _workspaces.append(ws);
            // Восстанавливаем SubspaceLinkItem
            if (parent) {
                for (const AbstractWorkspaceItem *item : ws->getItems()) {
                    if (item->type() == "SubspaceLinkItem") {
                        auto *link =
                         static_cast<SubspaceLinkItem *>(const_cast<AbstractWorkspaceItem *>(item));
                        linkItemsToFix.append({ link, link->serialize()["subspaceId"].toString() });
                        QObject::connect(link, &SubspaceLinkItem::subspaceLinkClicked, parent,
                                         &Workspace::subWorkspaceClicked);
                    }
                }
            }
            if (obj.contains("subWorkspaces")) {
                QJsonArray subArr = obj["subWorkspaces"].toArray();
                for (const QJsonValue &subVal : subArr) {
                    QString subId = subVal.toString();
                    // subWorkspaces будут добавлены после обхода всех
                }
            }
            return ws;
        };
        for (const QJsonValue &val : arr) {
            loadTree(val.toObject(), nullptr);
        }
        // Второй проход: связываем subWorkspaces
        for (Workspace *ws : _workspaces) {
            QJsonObject obj = ws->serialize();
            if (obj.contains("subWorkspaces")) {
                QJsonArray subArr = obj["subWorkspaces"].toArray();
                for (const QJsonValue &subVal : subArr) {
                    QString subId = subVal.toString();
                    if (idMap.contains(subId))
                        ws->addSubWorkspace(idMap[subId]);
                }
            }
        }
        // Связываем SubspaceLinkItem с Workspace
        for (auto &pair : linkItemsToFix) {
            if (idMap.contains(pair.second))
                pair.first->setLinkedWorkspace(idMap[pair.second]);
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

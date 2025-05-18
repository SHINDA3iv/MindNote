#include "workspace_controller.h"
#include <QUuid>
#include "../data/elements/SubspaceLinkItem.h"
#include <QDebug>
#include <QInputDialog>
#include <QLineEdit>

WorkspaceController::WorkspaceController(std::shared_ptr<LocalStorage> localStorage,
                                         QObject *parent) :
    QObject(parent),
    _localStorage(localStorage)
{
    loadWorkspaces();
    
    // Connect signals from loaded workspaces
    for (Workspace *ws : _workspaces) {
        connect(ws, &Workspace::addSubspaceRequested, this, [this, ws]() {
            handleAddSubspaceRequest(ws);
        });
    }
}

void WorkspaceController::handleAddSubspaceRequest(Workspace *parent)
{
    if (!parent) return;
    
    bool ok;
    QString name = QInputDialog::getText(nullptr, "Новое подпространство",
                                       "Введите название подпространства:",
                                       QLineEdit::Normal, "", &ok);
    if (ok && !name.isEmpty()) {
        createSubWorkspace(parent, name);
    }
}

Workspace *WorkspaceController::createWorkspace(const QString &name, const QString &id)
{
    QString workspaceId = id.isEmpty() ? QUuid::createUuid().toString() : id;
    Workspace *workspace = new Workspace(name);
    workspace->setProperty("id", workspaceId);

    // Set default workspace icon
    workspace->setIcon(QIcon(":/icons/workspace.png"));

    _workspaces.append(workspace);
    emit workspaceAdded(workspace);

    // Сохраняем изменения
    saveWorkspaces();

    return workspace;
}

void WorkspaceController::removeWorkspace(Workspace *workspace)
{
    if (!workspace)
        return;

    QString workspaceId = workspace->property("id").toString();
    Workspace* parent = workspace->getParentWorkspace();

    // Удаляем все подпространства
    QList<Workspace*> subspaces = workspace->getSubWorkspaces();
    for (Workspace* sub : subspaces) {
        removeWorkspace(sub);
    }

    // Удаляем ссылки на это пространство из родительского пространства
    if (parent) {
        parent->removeSubWorkspace(workspace);
        
        // Удаляем ссылки-элементы из родительского пространства
        QList<AbstractWorkspaceItem*> items = parent->getItems();
        for (AbstractWorkspaceItem* item : items) {
            if (item->type() == "SubspaceLinkItem") {
                auto* link = static_cast<SubspaceLinkItem*>(item);
                if (link->getLinkedWorkspace() == workspace) {
                    parent->removeItem(item);
                }
            }
        }
    }

    // Удаляем из списка и освобождаем память
    _workspaces.removeOne(workspace);
    emit workspaceRemoved(workspace);
    delete workspace;

    // Удаляем связанные элементы
    _localStorage->saveWorkspaceItems(workspaceId, QJsonArray());

    // Сохраняем изменения
    saveWorkspaces();

    // Открываем родительское пространство и обновляем путь
    if (parent) {
        emit workspaceAdded(parent);
        emit pathUpdated(parent);
    }
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
    emit workspaceAdded(sub);
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
    
    // Save all workspaces
    for (const Workspace *ws : _workspaces) {
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
        
        // First pass: Create all workspaces
        for (const QJsonValue &val : arr) {
            QJsonObject obj = val.toObject();
            QString name = obj["name"].toString();
            QString id = obj["id"].toString();
            
            Workspace *ws = new Workspace(name);
            ws->setId(id);
            idMap[id] = ws;
            _workspaces.append(ws);
            
            qDebug() << "Created workspace:" << name << "ID:" << id;
        }
        
        // Second pass: Set up parent-child relationships and deserialize content
        for (const QJsonValue &val : arr) {
            QJsonObject obj = val.toObject();
            QString id = obj["id"].toString();
            QString parentId = obj["parentId"].toString();
            
            Workspace *ws = idMap[id];
            if (!parentId.isEmpty() && idMap.contains(parentId)) {
                Workspace *parent = idMap[parentId];
                ws->setParentWorkspace(parent);
                parent->addSubWorkspace(ws);
                
                qDebug() << "Setting up relationship:" 
                         << "Child:" << ws->getName() 
                         << "Parent:" << parent->getName();
            }
            
            // Deserialize workspace content
            ws->deserialize(obj);
            
            // Handle SubspaceLinkItem connections
            for (const AbstractWorkspaceItem *item : ws->getItems()) {
                if (item->type() == "SubspaceLinkItem") {
                    auto *link = static_cast<SubspaceLinkItem *>(const_cast<AbstractWorkspaceItem *>(item));
                    QString subspaceId = link->serialize()["subspaceId"].toString();
                    if (idMap.contains(subspaceId)) {
                        link->setLinkedWorkspace(idMap[subspaceId]);
                        QObject::connect(link, &SubspaceLinkItem::subspaceLinkClicked, 
                                       ws, &Workspace::subWorkspaceClicked);
                        
                        qDebug() << "Setting up link:" 
                                << "From:" << ws->getName()
                                << "To:" << idMap[subspaceId]->getName();
                    }
                }
            }
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
        workspacesArray.append(workspaceObj);
        
        qDebug() << "Saving workspace:" << workspace->getName() 
                 << "ID:" << workspace->getId()
                 << "Parent:" << (workspace->getParentWorkspace() ? workspace->getParentWorkspace()->getId() : "none")
                 << "Subworkspaces:" << workspace->getSubWorkspaces().size();
    }

    _localStorage->saveWorkspaces(workspacesArray);
}

void WorkspaceController::loadWorkspaces()
{
    QJsonArray workspacesArray = _localStorage->loadWorkspaces();
    QMap<QString, Workspace*> idMap;

    // First pass: Create all workspaces
    for (const QJsonValue &workspaceVal : workspacesArray) {
        QJsonObject workspaceObj = workspaceVal.toObject();
        QString name = workspaceObj["name"].toString();
        QString id = workspaceObj["id"].toString();
        
        Workspace *workspace = new Workspace(name);
        workspace->setId(id);
        idMap[id] = workspace;
        _workspaces.append(workspace);
        
        qDebug() << "Created workspace:" << name << "ID:" << id;
    }

    // Second pass: Set up parent-child relationships and deserialize content
    for (const QJsonValue &workspaceVal : workspacesArray) {
        QJsonObject workspaceObj = workspaceVal.toObject();
        QString id = workspaceObj["id"].toString();
        QString parentId = workspaceObj["parentId"].toString();
        
        Workspace *workspace = idMap[id];
        if (!parentId.isEmpty() && idMap.contains(parentId)) {
            Workspace *parent = idMap[parentId];
            workspace->setParentWorkspace(parent);
            parent->addSubWorkspace(workspace);
            
            qDebug() << "Setting up relationship:" 
                     << "Child:" << workspace->getName() 
                     << "Parent:" << parent->getName();
        }
        
        // Deserialize workspace content
        workspace->deserialize(workspaceObj);
        
        // Handle SubspaceLinkItem connections
        for (const AbstractWorkspaceItem *item : workspace->getItems()) {
            if (item->type() == "SubspaceLinkItem") {
                auto *link = static_cast<SubspaceLinkItem *>(const_cast<AbstractWorkspaceItem *>(item));
                QString subspaceId = link->serialize()["subspaceId"].toString();
                if (idMap.contains(subspaceId)) {
                    link->setLinkedWorkspace(idMap[subspaceId]);
                    QObject::connect(link, &SubspaceLinkItem::subspaceLinkClicked, 
                                   workspace, &Workspace::subWorkspaceClicked);
                    
                    qDebug() << "Setting up link:" 
                            << "From:" << workspace->getName()
                            << "To:" << idMap[subspaceId]->getName();
                }
            }
        }
    }
}

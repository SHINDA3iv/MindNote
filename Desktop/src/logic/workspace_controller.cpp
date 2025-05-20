#include "workspace_controller.h"
#include <QUuid>
#include "../data/elements/SubspaceLinkItem.h"
#include <QDebug>
#include <QInputDialog>
#include <QLineEdit>
#include <qapplication.h>

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
    if (!parent)
        return;

    bool ok;
    QString name =
     QInputDialog::getText(nullptr, "Новое подпространство",
                           "Введите название подпространства:", QLineEdit::Normal, "", &ok);
    if (ok && !name.isEmpty()) {
        createSubWorkspace(parent, name);
    }
}

Workspace *WorkspaceController::createWorkspace(const QString &name, const QString &id)
{
    QString workspaceId = id.isEmpty() ? QUuid::createUuid().toString() : id;
    Workspace *workspace = new Workspace(name);
    workspace->setProperty("id", workspaceId);

    // Устанавливаем иконку по умолчанию
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

    Workspace *root = workspace->getRootWorkspace();
    Workspace *parent = workspace->getParentWorkspace();

    // Рекурсивное удаление подпространств
    QList<Workspace *> subspaces = workspace->getSubWorkspaces();
    for (Workspace *sub : subspaces) {
        removeWorkspace(sub);
    }

    // Удаление ссылок из родительского пространства
    if (parent) {
        parent->removeSubWorkspace(workspace);
    }

    // Удаление из списка корневых пространств, если это корневое
    if (!parent) {
        _workspaces.removeOne(workspace);
    }

    // Удаление физически
    _localStorage->deleteWorkspace(workspace->getName());

    // Если есть родительское пространство - сохраняем его после изменений
    if (parent) {
        _localStorage->saveWorkspace(root);
    }

    emit workspaceRemoved(workspace);
    delete workspace;
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

Workspace *WorkspaceController::createSubWorkspace(Workspace *parent, const QString &name)
{
    if (!parent)
        return nullptr;

    Workspace *subspace = new Workspace(name, parent);
    parent->addSubWorkspace(subspace);

    // Наследуем иконку от родителя или используем иконку по умолчанию
    QIcon parentIcon = parent->getIcon();
    if (!parentIcon.isNull()) {
        subspace->setIcon(parentIcon);
    } else {
        subspace->setIcon(QIcon(":/icons/workspace.png"));
    }

    // Сохраняем родительское пространство, чтобы сохранить структуру вложенности
    _localStorage->saveWorkspace(parent->getRootWorkspace());

    return subspace;
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
                         << "Child:" << ws->getName() << "Parent:" << parent->getName();
            }

            // Deserialize workspace content
            ws->deserialize(obj);

            // Handle SubspaceLinkItem connections
            for (const AbstractWorkspaceItem *item : ws->getItems()) {
                if (item->type() == "SubspaceLinkItem") {
                    auto *link =
                     static_cast<SubspaceLinkItem *>(const_cast<AbstractWorkspaceItem *>(item));
                    QString subspaceId = link->serialize()["subspaceId"].toString();
                    if (idMap.contains(subspaceId)) {
                        link->setLinkedWorkspace(idMap[subspaceId]);
                        QObject::connect(link, &SubspaceLinkItem::subspaceLinkClicked, ws,
                                         &Workspace::subWorkspaceClicked);

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
    for (Workspace *workspace : _workspaces) {
        if (!workspace->getParentWorkspace()) {
            _localStorage->saveWorkspace(workspace);
        }
    }
}

void WorkspaceController::loadWorkspaces()
{
    QDir workspacesDir(QApplication::applicationDirPath() + "/Workspaces/");
    if (!workspacesDir.exists())
        return;

    QFileInfoList folders = workspacesDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo &folder : folders) {
        Workspace *workspace = _localStorage->loadWorkspace(folder.baseName());
        if (workspace) {
            _workspaces.append(workspace);
        }
    }
}

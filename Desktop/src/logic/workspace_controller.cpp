#include "workspace_controller.h"
#include <QUuid>
#include "../data/elements/SubspaceLinkItem.h"
#include <QDebug>
#include <QInputDialog>
#include <QLineEdit>
#include <qapplication.h>

WorkspaceController::WorkspaceController(std::shared_ptr<LocalStorage> localStorage,
                                         QWidget *parent) :
    QObject(parent),
    _localStorage(localStorage)
{
    loadWorkspaces(parent);

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
    QString title =
     QInputDialog::getText(nullptr, "Новое подпространство",
                           "Введите название подпространства:", QLineEdit::Normal, "", &ok);
    if (ok && !title.isEmpty()) {
        createSubWorkspace(parent, title);
    }
}

Workspace *WorkspaceController::createWorkspace(const QString &title)
{
    Workspace *workspace = new Workspace(title);
    workspace->setProperty("title", title);

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
    _localStorage->deleteWorkspace(workspace->getTitle());

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

Workspace *WorkspaceController::getWorkspaceByTitle(const QString &title) const
{
    for (Workspace *workspace : _workspaces) {
        if (workspace->property("title").toString() == title) {
            return workspace;
        }
    }
    return nullptr;
}

Workspace *WorkspaceController::createSubWorkspace(Workspace *parent, const QString &title)
{
    if (!parent)
        return nullptr;

    Workspace *subspace = new Workspace(title, parent);
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

Workspace *WorkspaceController::findWorkspaceByTitle(const QString &title) const
{
    for (Workspace *ws : _workspaces) {
        if (ws->getTitle() == title)
            return ws;
    }
    return nullptr;
}

QJsonObject WorkspaceController::serialize() const
{
    QJsonObject json;
    QJsonArray workspacesArray;
    for (const Workspace *ws : _workspaces) {
        if (!ws->getParentWorkspace()) {
            workspacesArray.append(ws->serializeBackend(true));
        }
    }
    json["workspaces"] = workspacesArray;
    return json;
}

void WorkspaceController::deserialize(const QJsonObject &json)
{
    qDeleteAll(_workspaces);
    _workspaces.clear();
    if (json.contains("workspaces")) {
        QJsonArray arr = json["workspaces"].toArray();
        for (const QJsonValue &val : arr) {
            QJsonObject obj = val.toObject();
            Workspace *ws = new Workspace();
            ws->deserializeBackend(obj, true);
            _workspaces.append(ws);
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
    QString currentUser = _localStorage->getCurrentUser();
    bool isGuest = currentUser.isEmpty();

    for (Workspace *workspace : _workspaces) {
        if (!workspace->getParentWorkspace()) {
            _localStorage->saveWorkspace(workspace, isGuest);
        }
    }
}

void WorkspaceController::loadWorkspaces(QWidget *parent)
{
    QString currentUser = _localStorage->getCurrentUser();

    if (currentUser.isEmpty()) {
        // Load guest workspaces only if not authenticated
        QDir guestDir(QApplication::applicationDirPath() + "/Workspaces/guest/");
        if (guestDir.exists()) {
            QFileInfoList folders = guestDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
            for (const QFileInfo &folder : folders) {
                Workspace *workspace =
                 _localStorage->loadWorkspace(folder.baseName(), parent, true);
                if (workspace) {
                    _workspaces.append(workspace);
                }
            }
        }
    } else {
        // Load user workspaces if authenticated
        QDir userDir(QApplication::applicationDirPath() + "/Workspaces/users/" + currentUser + "/");
        if (userDir.exists()) {
            QFileInfoList folders = userDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
            for (const QFileInfo &folder : folders) {
                Workspace *workspace =
                 _localStorage->loadWorkspace(folder.baseName(), parent, false);
                if (workspace) {
                    _workspaces.append(workspace);
                }
            }
        }
    }
}

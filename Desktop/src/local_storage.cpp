#include "local_storage.h"
#include <QJsonDocument>
#include <QFile>
#include <qapplication.h>
#include <qbuffer.h>
#include <QDebug>
#include <QMessageBox>

LocalStorage::LocalStorage(QObject *parent) : QObject(parent)
{
    initializePaths();
}

void LocalStorage::initializePaths()
{
    storagePath = QApplication::applicationDirPath() + "/Workspaces/";
    guestPath = storagePath + "guest/";
    userPath = storagePath + "users/";

    QDir().mkpath(guestPath);
    QDir().mkpath(userPath);
}

QString LocalStorage::getWorkspacePath(bool isGuest) const
{
    return isGuest ? guestPath : getUserWorkspacePath();
}

QString LocalStorage::getUserWorkspacePath() const
{
    if (currentUser.isEmpty()) {
        return userPath;
    }
    return userPath + currentUser + "/";
}

QString LocalStorage::getWorkspaceOwnerPath(const QString &ownerUsername) const
{
    return userPath + ownerUsername + "/";
}

void LocalStorage::setCurrentUser(const QString &username)
{
    currentUser = username;
    QDir().mkpath(getUserWorkspacePath());
}

QString LocalStorage::getCurrentUser() const
{
    return currentUser;
}

void LocalStorage::clearUserData()
{
    if (!currentUser.isEmpty()) {
        QDir userDir(getUserWorkspacePath());
        if (userDir.exists()) {
            userDir.removeRecursively();
        }
    }
    currentUser.clear();
}

void LocalStorage::saveWorkspace(Workspace *workspace, bool isGuest)
{
    if (!workspace)
        return;

    QString ownerUsername = workspace->getOwner();
    QString workspacePath;

    if (isGuest) {
        workspacePath = guestPath + workspace->getTitle() + "/";
    } else {
        workspacePath = getUserWorkspacePath() + workspace->getTitle() + "/";
    }

    QDir().mkpath(workspacePath);

    QFile file(workspacePath + "workspace.json");
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open workspace file for writing:" << file.errorString();
        return;
    }

    QJsonObject json;
    saveWorkspaceRecursive(workspace, json);
    QByteArray jsonData = QJsonDocument(json).toJson();

    if (file.write(jsonData) == -1) {
        qWarning() << "Failed to write workspace data:" << file.errorString();
    }
    file.close();
}

void LocalStorage::saveWorkspaceRecursive(Workspace *workspace, QJsonObject &json)
{
    json["title"] = workspace->getTitle();

    qDebug() << "Saving workspace recursively:" << workspace->getTitle();

    // Сохранение иконки
    QIcon icon = workspace->getIcon();
    if (!icon.isNull()) {
        QByteArray iconData;
        QBuffer buffer(&iconData);
        buffer.open(QIODevice::WriteOnly);

        // Сохраняем первую доступную pixmap
        QPixmap pixmap = icon.pixmap(icon.availableSizes().first());
        if (pixmap.save(&buffer, "PNG")) {
            json["icon"] = QString::fromLatin1(iconData.toBase64().data());
            qDebug() << "Saved icon for workspace:" << workspace->getTitle();
        }
    }

    // Save workspace items
    QJsonArray itemsArray;
    for (const AbstractWorkspaceItem *item : workspace->getItems()) {
        itemsArray.append(item->serialize());
        qDebug() << "Saved item of type:" << item->type()
                 << "in workspace:" << workspace->getTitle();
    }
    json["items"] = itemsArray;

    // Save subspaces
    QJsonArray subspaces;
    for (Workspace *sub : workspace->getSubWorkspaces()) {
        QJsonObject subJson;
        saveWorkspaceRecursive(sub, subJson);
        subspaces.append(subJson);
        qDebug() << "Saved subspace:" << sub->getTitle()
                 << "for workspace:" << workspace->getTitle();
    }
    json["subspaces"] = subspaces;
}

Workspace *LocalStorage::loadWorkspace(const QString &workspaceTitle, QWidget *parent, bool isGuest)
{
    QString workspacePath = getWorkspacePath(isGuest) + workspaceTitle + "/workspace.json";
    QFile file(workspacePath);

    if (!file.exists()) {
        qWarning() << "Workspace file not found:" << workspacePath;
        return nullptr;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open workspace file for reading:" << file.errorString();
        return nullptr;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid JSON in workspace file:" << workspacePath;
        return nullptr;
    }

    return loadWorkspaceRecursive(doc.object(), parent);
}

Workspace *LocalStorage::loadWorkspaceRecursive(const QJsonObject &json, QWidget *parent)
{
    Workspace *workspace = new Workspace(json["title"].toString(), parent);

    qDebug() << "Loading workspace recursively:" << workspace->getTitle();

    // Загрузка иконки
    if (json.contains("icon")) {
        QByteArray iconData = QByteArray::fromBase64(json["icon"].toString().toLatin1());
        QPixmap pixmap;
        if (pixmap.loadFromData(iconData)) {
            workspace->setIcon(QIcon(pixmap));
            qDebug() << "Loaded icon for workspace:" << workspace->getTitle();
        }
    } else {
        // Установка иконки по умолчанию если не задана пользовательская
        workspace->setIcon(QIcon(":/icons/workspace.png"));
    }

    // Load workspace items
    if (json.contains("items")) {
        QJsonArray itemsArray = json["items"].toArray();
        workspace->deserializeItems(itemsArray);
        qDebug() << "Loaded" << itemsArray.size()
                 << "items for workspace:" << workspace->getTitle();
    }

    // Load subspaces
    if (json.contains("subspaces")) {
        QJsonArray subspaceArray = json["subspaces"].toArray();
        for (const QJsonValue &value : subspaceArray) {
            Workspace *sub = loadWorkspaceRecursive(value.toObject(), workspace);
            if (sub) {
                workspace->addSubWorkspace(sub);
                qDebug() << "Loaded subspace:" << sub->getTitle()
                         << "for workspace:" << workspace->getTitle();
            }
        }
    }

    return workspace;
}

void LocalStorage::deleteWorkspace(const QString &workspaceTitle, bool isGuest)
{
    QString workspacePath = getWorkspacePath(isGuest) + workspaceTitle;
    QDir dir(workspacePath);
    if (dir.exists()) {
        dir.removeRecursively();
    }
}

void LocalStorage::syncWorkspaces(const QJsonArray &serverWorkspaces, bool keepLocal)
{
    // Get list of local workspaces
    QDir guestDir(guestPath);
    QDir userDir(getUserWorkspacePath());

    QStringList localWorkspaces;
    if (keepLocal) {
        localWorkspaces = guestDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    }

    // Process server workspaces
    for (const QJsonValue &workspaceValue : serverWorkspaces) {
        QJsonObject workspaceObj = workspaceValue.toObject();
        QString workspaceTitle = workspaceObj["title"].toString();
        QString version = workspaceObj["version"].toString();
        QString owner = workspaceObj["owner"].toString();

        // Check if workspace exists locally
        bool existsLocally = localWorkspaces.contains(workspaceTitle);

        if (existsLocally && keepLocal) {
            // Keep local version
            continue;
        }

        // Create or update workspace
        Workspace *workspace = new Workspace(workspaceTitle);
        workspace->setVersion(version);
        workspace->setOwner(owner);

        // Load workspace items if any
        if (workspaceObj.contains("items")) {
            QJsonArray items = workspaceObj["items"].toArray();
            workspace->deserializeItems(items);
        }

        // Save workspace to user's directory
        saveWorkspace(workspace, false);
        delete workspace;
    }

    // Remove workspaces that don't exist on server
    if (!keepLocal) {
        QStringList serverWorkspaceTitles;
        for (const QJsonValue &workspaceValue : serverWorkspaces) {
            QJsonObject workspaceObj = workspaceValue.toObject();
            serverWorkspaceTitles.append(workspaceObj["title"].toString());
        }

        QStringList localWorkspaceTitles = userDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &workspaceTitle : localWorkspaceTitles) {
            if (!serverWorkspaceTitles.contains(workspaceTitle)) {
                deleteWorkspace(workspaceTitle, false);
            }
        }
    }
}

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
    QDir userDir(userPath);
    if (userDir.exists()) {
        userDir.removeRecursively();
        QDir().mkpath(userPath); // пересоздать пустую папку users
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

    // Выводим содержимое json-файла после записи
    qDebug() << "[LocalStorage] Saved workspace to" << file.fileName();
    qDebug().noquote() << QJsonDocument::fromJson(jsonData).toJson(QJsonDocument::Indented);
}

void LocalStorage::saveWorkspaceRecursive(Workspace *workspace, QJsonObject &json)
{
    json = workspace->serializeBackend();
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
    workspace->deserializeBackend(json);
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
    QDir userDir(getUserWorkspacePath());
    QStringList localWorkspaceTitles = userDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    QStringList serverWorkspaceTitles;
    for (const QJsonValue &workspaceValue : serverWorkspaces) {
        QJsonObject workspaceObj = workspaceValue.toObject();
        QString workspaceTitle = workspaceObj["title"].toString();
        serverWorkspaceTitles.append(workspaceTitle);

        // Полная замена: всегда перезаписываем локальные данные
        Workspace *workspace = new Workspace(workspaceTitle);
        workspace->deserializeBackend(workspaceObj, true);
        saveWorkspace(workspace, false);
        delete workspace;
    }

    // Удаляем локальные workspaces, которых нет на сервере (если не keepLocal)
    if (!keepLocal) {
        for (const QString &workspaceTitle : localWorkspaceTitles) {
            if (!serverWorkspaceTitles.contains(workspaceTitle)) {
                deleteWorkspace(workspaceTitle, false);
            }
        }
    }
}

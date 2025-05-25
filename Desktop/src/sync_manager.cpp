#include "sync_manager.h"
#include "api/api_client.h"
#include "local_storage.h"
#include <QJsonArray>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include "user_sync_dialog.h"

SyncManager::SyncManager(std::shared_ptr<ApiClient> apiClient,
                         std::shared_ptr<LocalStorage> localStorage,
                         QObject *parent) :
    QObject(parent),
    apiClient(apiClient),
    localStorage(localStorage)
{
    connect(apiClient.get(), &ApiClient::workspacesReceived, this,
            &SyncManager::onWorkspacesFetched);
    connect(apiClient.get(), &ApiClient::pagesReceived, this, &SyncManager::onPagesFetched);
    connect(apiClient.get(), &ApiClient::syncCompleted, this, &SyncManager::onSyncCompleted);
    connect(apiClient.get(), &ApiClient::error, this, &SyncManager::onError);
    connect(&_syncTimer, &QTimer::timeout, this, &SyncManager::syncLocalChanges);
    connect(apiClient.get(), &ApiClient::userSyncDiffReceived, this, &SyncManager::onUserSyncDiffReceived);
    connect(apiClient.get(), &ApiClient::userSyncFinalReceived, this, &SyncManager::onUserSyncFinalReceived);
}

void SyncManager::startAutoSync(int intervalMs)
{
    _syncTimer.start(intervalMs);
}

void SyncManager::stopAutoSync()
{
    _syncTimer.stop();
}

void SyncManager::performFullSync()
{
    if (_isSyncing)
        return;

    _isSyncing = true;
    emit syncStarted();
    apiClient->getWorkspaces();
}

void SyncManager::syncWithVersionSelection(const QJsonArray &serverWorkspaces)
{
    if (_isSyncing)
        return;

    _isSyncing = true;
    emit syncStarted();

    if (hasVersionConflicts(serverWorkspaces)) {
        emit versionConflictDetected(serverWorkspaces);
        return;
    }

    // No conflicts, proceed with sync
    localStorage->syncWorkspaces(serverWorkspaces, false);
    _isSyncing = false;
    emit syncCompleted();
}

bool SyncManager::hasVersionConflicts(const QJsonArray &serverWorkspaces)
{
    QDir userDir(localStorage->getWorkspacePath(false));
    QStringList localWorkspaces = userDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &workspaceName : localWorkspaces) {
        QString localPath = userDir.filePath(workspaceName) + "/workspace.json";
        QFile localFile(localPath);
        
        if (localFile.open(QIODevice::ReadOnly)) {
            QJsonDocument localDoc = QJsonDocument::fromJson(localFile.readAll());
            localFile.close();
            
            if (localDoc.isObject()) {
                QJsonObject localWorkspace = localDoc.object();
                QString localVersion = localWorkspace["version"].toString();
                
                // Find corresponding server workspace
                for (const QJsonValue &serverValue : serverWorkspaces) {
                    QJsonObject serverWorkspace = serverValue.toObject();
                    if (serverWorkspace["name"].toString() == workspaceName) {
                        QString serverVersion = serverWorkspace["version"].toString();
                        if (localVersion != serverVersion) {
                            return true;
                        }
                        break;
                    }
                }
            }
        }
    }
    
    return false;
}

void SyncManager::onWorkspacesFetched(const QJsonArray &workspaces)
{
    if (hasVersionConflicts(workspaces)) {
        emit versionConflictDetected(workspaces);
        return;
    }

    localStorage->syncWorkspaces(workspaces, false);
    for (const QJsonValue &workspace : workspaces) {
        QString workspaceTitle = workspace.toObject()["title"].toString();
        apiClient->getPages(workspaceTitle);
    }
}

void SyncManager::onPagesFetched(const QString &workspaceTitle, const QJsonArray &pages)
{
    // Update workspace items in local storage
    QDir userDir(localStorage->getWorkspacePath(false));
    QString workspacePath = userDir.filePath(workspaceTitle) + "/workspace.json";
    QFile workspaceFile(workspacePath);
    if (workspaceFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(workspaceFile.readAll());
        workspaceFile.close();
        if (doc.isObject()) {
            QJsonObject workspace = doc.object();
            workspace["pages"] = pages;
            if (workspaceFile.open(QIODevice::WriteOnly)) {
                workspaceFile.write(QJsonDocument(workspace).toJson());
                workspaceFile.close();
            }
        }
    }
}

void SyncManager::onSyncCompleted(const QJsonObject &response)
{
    _isSyncing = false;
    emit syncCompleted();
}

void SyncManager::onError(const QString &message)
{
    _isSyncing = false;
    emit syncError(message);
    qWarning() << "Sync error:" << message;
}

void SyncManager::syncLocalChanges()
{
    if (_isSyncing)
        return;

    _isSyncing = true;
    emit syncStarted();
    QJsonObject changes = collectLocalChanges();
    QDir userDir(localStorage->getWorkspacePath(false));
    QStringList workspaceNames = userDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &workspaceName : workspaceNames) {
        apiClient->syncWorkspace(workspaceName, changes);
    }
}

QJsonObject SyncManager::collectLocalChanges()
{
    QJsonObject changes;
    QJsonArray workspaces;
    
    QDir userDir(localStorage->getWorkspacePath(false));
    QStringList workspaceNames = userDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    
    for (const QString &workspaceName : workspaceNames) {
        QString workspacePath = userDir.filePath(workspaceName) + "/workspace.json";
        QFile workspaceFile(workspacePath);
        
        if (workspaceFile.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(workspaceFile.readAll());
            workspaceFile.close();
            
            if (doc.isObject()) {
                workspaces.append(doc.object());
            }
        }
    }
    
    changes["workspaces"] = workspaces;
    return changes;
}

void SyncManager::startUserSync()
{
    // Собрать все локальные guest workspaces
    QJsonArray localWorkspaces;
    QDir guestDir(localStorage->getWorkspacePath(true));
    QStringList workspaceNames = guestDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &workspaceName : workspaceNames) {
        QString workspacePath = guestDir.filePath(workspaceName) + "/workspace.json";
        QFile workspaceFile(workspacePath);
        if (workspaceFile.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(workspaceFile.readAll());
            workspaceFile.close();
            if (doc.isObject()) {
                localWorkspaces.append(doc.object());
            }
        }
    }
    apiClient->postUserSync(localWorkspaces);
}

void SyncManager::applyUserSyncResolution(const QJsonArray &resolve, const QJsonArray &newWorkspaces)
{
    apiClient->patchUserSync(resolve, newWorkspaces);
}

void SyncManager::onUserSyncDiffReceived(const QJsonObject &diff)
{
    UserSyncDialog dlg(diff);
    if (dlg.exec() == QDialog::Accepted) {
        QJsonArray resolve = dlg.getResolve();
        QJsonArray newWorkspaces = dlg.getNewWorkspaces();
        applyUserSyncResolution(resolve, newWorkspaces);
    }
}

void SyncManager::onUserSyncFinalReceived(const QJsonArray &finalWorkspaces)
{
    // 1. Сохраняем все workspaces пользователя локально (очищаем старые)
    localStorage->syncWorkspaces(finalWorkspaces, true); // true = полная замена

    // 2. Очищаем guest workspaces
    QDir guestDir(localStorage->getWorkspacePath(true));
    QStringList guestWorkspaces = guestDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &ws : guestWorkspaces) {
        guestDir.rmdir(ws);
    }

    // 3. Загружаем страницы для каждого workspace с сервера
    for (const QJsonValue &wsVal : finalWorkspaces) {
        QJsonObject ws = wsVal.toObject();
        QString title = ws["title"].toString();
        apiClient->getPages(title);
    }

    emit syncCompleted();
}

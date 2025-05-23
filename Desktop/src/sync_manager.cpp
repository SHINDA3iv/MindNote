#include "sync_manager.h"
#include "api/api_client.h"
#include "local_storage.h"
#include <QJsonArray>
#include <QMessageBox>

SyncManager::SyncManager(std::shared_ptr<ApiClient> apiClient,
                         std::shared_ptr<LocalStorage> localStorage,
                         QObject *parent) :
    QObject(parent),
    apiClient(apiClient),
    localStorage(localStorage)
{
    connect(apiClient.get(), &ApiClient::workspacesReceived, this,
            &SyncManager::onWorkspacesFetched);
    connect(apiClient.get(), &ApiClient::itemsReceived, this, &SyncManager::onItemsFetched);
    connect(apiClient.get(), &ApiClient::syncCompleted, this, &SyncManager::onSyncCompleted);
    connect(apiClient.get(), &ApiClient::error, this, &SyncManager::onError);
    connect(&_syncTimer, &QTimer::timeout, this, &SyncManager::syncLocalChanges);
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
    // TODO: Implement version conflict detection logic
    // For now, always return false
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
        QString workspaceId = workspace.toObject()["id"].toString();
        apiClient->getWorkspaceItems(workspaceId);
    }
}

void SyncManager::onItemsFetched(const QString &workspaceId, const QJsonArray &items)
{
    // TODO: Implement workspace items synchronization
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
    apiClient->syncChanges(changes);
}

QJsonObject SyncManager::collectLocalChanges()
{
    QJsonObject changes;
    // TODO: Implement local changes collection
    return changes;
}

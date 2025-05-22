#include "sync_manager.h"
#include "api/api_client.h"
#include "local_storage.h"

#include <QJsonArray>

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
    apiClient->getWorkspaces();
}

void SyncManager::onWorkspacesFetched(const QJsonArray &workspaces)
{
    // localStorage->saveWorkspaces(workspaces);
    // for (const QJsonValue &workspace : workspaces) {
    //     QString workspaceId = workspace.toObject()["id"].toString();
    //     apiClient->getWorkspaceItems(workspaceId);
    // }
}

void SyncManager::onItemsFetched(const QString &workspaceId, const QJsonArray &items)
{
    // localStorage->saveWorkspaceItems(workspaceId, items);
}

void SyncManager::onSyncCompleted(const QJsonObject &response)
{
    // _isSyncing = false;
    // localStorage->setLastSyncTime(QDateTime::currentDateTime());

    // performFullSync();
}

void SyncManager::onError(const QString &message)
{
    _isSyncing = false;
    qWarning() << "Sync error:" << message;
}

void SyncManager::syncLocalChanges()
{
    if (_isSyncing)
        return;

    _isSyncing = true;
    QJsonObject changes = collectLocalChanges();
    apiClient->syncChanges(changes);
}

QJsonObject SyncManager::collectLocalChanges()
{
    QJsonObject changes;

    // QJsonArray workspaces = localStorage->loadWorkspaces();
    // changes["workspaces"] = workspaces;

    // for (const QJsonValue &workspace : workspaces) {
    //     QString workspaceId = workspace.toObject()["id"].toString();
    //     QJsonArray items = localStorage->loadWorkspaceItems(workspaceId);
    //     changes[workspaceId + "_items"] = items;
    // }

    return changes;
}

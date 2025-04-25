// syncmanager.h
#ifndef SYNCMANAGER_H
#define SYNCMANAGER_H

#include <QObject>
#include "api/api_client.h"
#include "local_storage.h"
#include <QTimer>

class SyncManager : public QObject
{
    Q_OBJECT
public:
    explicit SyncManager(std::shared_ptr<ApiClient> apiClient,
                         std::shared_ptr<LocalStorage> localStorage,
                         QObject *parent = nullptr);

    void startAutoSync(int intervalMs);
    void stopAutoSync();
    void performFullSync();

private slots:
    void onWorkspacesFetched(const QJsonArray &workspaces);
    void onItemsFetched(const QString &workspaceId, const QJsonArray &items);
    void onSyncCompleted(const QJsonObject &response);
    void onError(const QString &message);

private:
    void syncLocalChanges();
    QJsonObject collectLocalChanges();

    std::shared_ptr<ApiClient> _apiClient { nullptr };
    std::shared_ptr<LocalStorage> _localStorage { nullptr };
    QTimer _syncTimer;
    bool _isSyncing { false };
};

#endif // SYNCMANAGER_H

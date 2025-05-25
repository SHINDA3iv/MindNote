// syncmanager.h
#ifndef SYNC_MANAGER_H
#define SYNC_MANAGER_H

#include <QObject>
#include <QTimer>
#include <memory>

class ApiClient;
class LocalStorage;

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
    void syncWithVersionSelection(const QJsonArray &serverWorkspaces);
    void startUserSync();
    void applyUserSyncResolution(const QJsonArray &resolve, const QJsonArray &newWorkspaces);

signals:
    void syncStarted();
    void syncCompleted();
    void syncError(const QString &error);
    void versionConflictDetected(const QJsonArray &serverWorkspaces);
    void userSyncDiffReceived(const QJsonObject &diff);
    void userSyncFinalReceived(const QJsonArray &finalWorkspaces);

private slots:
    void onWorkspacesFetched(const QJsonArray &workspaces);
    void onPagesFetched(const QString &workspaceTitle, const QJsonArray &pages);
    void onSyncCompleted(const QJsonObject &response);
    void onError(const QString &message);
    void syncLocalChanges();
    void onUserSyncDiffReceived(const QJsonObject &diff);
    void onUserSyncFinalReceived(const QJsonArray &finalWorkspaces);

private:
    QJsonObject collectLocalChanges();
    bool hasVersionConflicts(const QJsonArray &serverWorkspaces);

    std::shared_ptr<ApiClient> apiClient;
    std::shared_ptr<LocalStorage> localStorage;
    QTimer _syncTimer;
    bool _isSyncing = false;
};

#endif // SYNC_MANAGER_H

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

signals:
    void syncStarted();
    void syncCompleted();
    void syncError(const QString &error);
    void versionConflictDetected(const QJsonArray &serverWorkspaces);

private slots:
    void onWorkspacesFetched(const QJsonArray &workspaces);
    void onPagesFetched(const QString &workspaceTitle, const QJsonArray &pages);
    void onSyncCompleted(const QJsonObject &response);
    void onError(const QString &message);
    void syncLocalChanges();

private:
    QJsonObject collectLocalChanges();
    bool hasVersionConflicts(const QJsonArray &serverWorkspaces);

    std::shared_ptr<ApiClient> apiClient;
    std::shared_ptr<LocalStorage> localStorage;
    QTimer _syncTimer;
    bool _isSyncing = false;
};

#endif // SYNC_MANAGER_H

#ifndef APICLIENT_H
#define APICLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QPointer>

class ApiClient : public QObject
{
    Q_OBJECT
public:
    explicit ApiClient(QObject *parent = nullptr);

    void getWorkspaces();
    void createWorkspace(const QJsonObject &data);
    void updateWorkspace(int id, const QJsonObject &data);
    void deleteWorkspace(int id);

    void getWorkspaceItems(const QString &workspaceId);
    void createWorkspaceItem(const QString &workspaceId, const QJsonObject &data);
    void
    updateWorkspaceItem(const QString &workspaceId, const QString &itemId, const QJsonObject &data);
    void deleteWorkspaceItem(const QString &workspaceId, const QString &itemId);

    void syncChanges(const QJsonObject &changes);

signals:
    void workspacesReceived(const QJsonArray &workspaces);
    void workspaceCreated(const QJsonObject &workspace);
    void workspaceUpdated(const QJsonObject &workspace);
    void workspaceDeleted(const QString &id);

    void itemsReceived(const QString &workspaceId, const QJsonArray &items);
    void itemCreated(const QString &workspaceId, const QJsonObject &item);
    void itemUpdated(const QString &workspaceId, const QJsonObject &item);
    void itemDeleted(const QString &workspaceId, const QString &itemId);

    void syncCompleted(const QJsonObject &response);
    void errorOccurred(const QString &message);

public:
    void login(const QString &email, const QString &password);
    void registerUser(const QString &email, const QString &password, const QString &username);

    void setAuthToken(const QString &token);
    bool isAuthenticated() const;

signals:
    void loginSuccess(const QString &token);
    void loginFailed(const QString &error);
    void registrationSuccess();
    void registrationFailed(const QString &error);

private slots:
    void handleReply(QNetworkReply *reply);

private:
    QString _authToken;

    std::unique_ptr<QNetworkAccessManager> _manager { nullptr };
    QMap<QNetworkReply *, QString> _pendingRequests;
    QString _baseUrl { "https://localhost:8000/api/" };
};

#endif // APICLIENT_H

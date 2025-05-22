#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QUrl>

class ApiClient : public QObject
{
    Q_OBJECT

public:
    explicit ApiClient(QObject *parent = nullptr);
    ~ApiClient();

    // Базовые методы для работы с API
    void setBaseUrl(const QString &url);
    void setAuthToken(const QString &token);
    bool isAuthenticated() const;
    
    // Методы для работы с API
    void login(const QString &username, const QString &password);
    void registerUser(const QString &username, const QString &email, const QString &password);
    void getWorkspaces();
    void createWorkspace(const QString &name, const QString &description);
    void updateWorkspace(int id, const QString &name, const QString &description);
    void deleteWorkspace(int id);

    void getWorkspaceItems(const QString &workspaceId);
    void createWorkspaceItem(const QString &workspaceId, const QJsonObject &data);
    void updateWorkspaceItem(const QString &workspaceId, const QString &itemId, const QJsonObject &data);
    void deleteWorkspaceItem(const QString &workspaceId, const QString &itemId);

    void syncChanges(const QJsonObject &changes);
    void getCurrentUser();
    void updateUser(const QJsonObject &userData);
    void deleteUser();

signals:
    void loginSuccess(const QString &token);
    void loginError(const QString &error);
    void registerSuccess();
    void registerError(const QString &error);
    void workspacesReceived(const QJsonArray &workspaces);
    void workspaceCreated(const QJsonObject &workspace);
    void workspaceUpdated(const QJsonObject &workspace);
    void workspaceDeleted(int id);
    void error(const QString &error);

    void itemsReceived(const QString &workspaceId, const QJsonArray &items);
    void itemCreated(const QString &workspaceId, const QJsonObject &item);
    void itemUpdated(const QString &workspaceId, const QJsonObject &item);
    void itemDeleted(const QString &workspaceId, const QString &itemId);

    void syncCompleted(const QJsonObject &response);

private:
    QNetworkAccessManager *networkManager;
    QString baseUrl;
    QString authToken;

    QNetworkRequest createRequest(const QString &endpoint);
    void handleResponse(QNetworkReply *reply, const std::function<void(const QJsonDocument &)> &successCallback);
};

#endif // API_CLIENT_H

#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QUrl>
#include <functional>

class ApiClient : public QObject
{
    Q_OBJECT

public:
    explicit ApiClient(QObject *parent = nullptr);
    ~ApiClient();

    // Аутентификация
    void login(const QString &username, const QString &password);
    void registerUser(const QString &email, const QString &password, const QString &username);
    void logout();
    bool isAuthenticated() const;
    QString getUsername() const;
    void setBaseUrl(const QString &url);
    void setAuthToken(const QString &token);
    void validateToken();

    // Рабочие пространства
    void getWorkspaces();
    void getWorkspace(const QString &workspaceTitle);
    void createWorkspace(const QString &name, const QString &description);
    void updateWorkspace(int id, const QString &name, const QString &description);
    void deleteWorkspace(int id);

    // Страницы
    void getPages(const QString &workspaceTitle);
    void getPage(const QString &workspaceTitle, const QString &pageTitle);
    void createPage(const QString &workspaceTitle, const QJsonObject &page);
    void updatePage(const QString &workspaceTitle, const QString &pageTitle, const QJsonObject &page);
    void deletePage(const QString &workspaceTitle, const QString &pageTitle);

    // Элементы
    void getWorkspaceItems(const QString &workspaceTitle);
    void createWorkspaceItem(const QString &workspaceId, const QJsonObject &data);
    void updateWorkspaceItem(const QString &workspaceId, const QString &itemId, const QJsonObject &data);
    void deleteWorkspaceItem(const QString &workspaceId, const QString &itemId);

    // Синхронизация
    void syncWorkspace(const QString &workspaceTitle, const QJsonObject &workspace);
    void syncGuestWorkspaces();
    void syncUserWorkspaces(bool copyGuestWorkspaces = false);
    void syncChanges(const QJsonObject &changes);

    // Пользователь
    void getCurrentUser();
    void updateUser(const QJsonObject &userData);
    void deleteUser();

signals:
    void error(const QString &message);
    void loginSuccess(const QString &token);
    void loginError(const QString &message);
    void registerSuccess();
    void registerError(const QString &message);
    void logoutSuccess();
    void workspacesReceived(const QJsonArray &workspaces);
    void workspaceReceived(const QJsonObject &workspace);
    void workspaceCreated(const QJsonObject &workspace);
    void workspaceUpdated(const QJsonObject &workspace);
    void workspaceDeleted(int id);
    void pagesReceived(const QString &workspaceTitle, const QJsonArray &pages);
    void pageReceived(const QString &workspaceTitle, const QJsonObject &page);
    void itemsReceived(const QString &workspaceTitle, const QJsonArray &items);
    void itemCreated(const QString &workspaceTitle, const QJsonObject &item);
    void itemUpdated(const QString &workspaceTitle, const QJsonObject &item);
    void itemDeleted(const QString &workspaceTitle, const QString &itemId);
    void syncComplete(const QString &workspaceTitle);
    void syncCompleted(const QJsonObject &response);
    void guestWorkspacesSynced(const QJsonArray &workspaces);
    void userWorkspacesSynced(const QJsonArray &workspaces, const QJsonArray &conflicts);
    void tokenValid();
    void tokenInvalid();

private:
    QNetworkAccessManager *networkManager;
    QString token;
    QString username;
    QString baseUrl;
    QString authToken;
    QString _username;

    QNetworkRequest createRequest(const QString &endpoint) const;
    void handleResponse(QNetworkReply *reply, std::function<void(const QJsonDocument &)> callback);
    void handleError(QNetworkReply *reply);
};

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

    // Базовые методы
    void setBaseUrl(const QString &url);
    void setAuthToken(const QString &token);
    bool isAuthenticated() const;
    QString getUsername() const;

    // Аутентификация
    void login(const QString &username, const QString &password);
    void registerUser(const QString &email, const QString &password, const QString &username);
    void logout();

    // Рабочие пространства
    void getWorkspaces();
    void createWorkspace(const QJsonObject &workspaceData);
    void updateWorkspace(const QString &title, const QJsonObject &workspaceData);
    void deleteWorkspace(const QString &title);

    // Страницы
    void getPages(const QString &workspaceTitle);
    void createPage(const QString &workspaceTitle, const QString &title, bool isMain = false);
    void updatePage(const QString &workspaceTitle, const QString &title, const QString &newTitle, bool isMain);
    void deletePage(const QString &workspaceTitle, const QString &title);

    // Элементы страниц
    void addElement(const QString &workspaceTitle, const QString &pageTitle, const QJsonObject &element);
    void updateElement(const QString &workspaceTitle, const QString &pageTitle, const QString &elementId, const QJsonObject &element);
    void removeElement(const QString &workspaceTitle, const QString &pageTitle, const QString &elementId);

    // Синхронизация
    void syncWorkspace(const QString &workspaceTitle, const QJsonObject &changes);
    void syncUserWorkspaces();

    // Пользователь
    void getCurrentUser();
    void updateUser(const QJsonObject &userData);
    void deleteUser();

signals:
    // Аутентификация
    void loginSuccess(const QString &token);
    void loginError(const QString &error);
    void registerSuccess();
    void registerError(const QString &error);
    void logoutSuccess();

    // Рабочие пространства
    void workspacesReceived(const QJsonArray &workspaces);
    void workspaceCreated(const QJsonObject &workspace);
    void workspaceUpdated(const QJsonObject &workspace);
    void workspaceDeleted(const QString &title);

    // Страницы
    void pagesReceived(const QString &workspaceTitle, const QJsonArray &pages);
    void pageCreated(const QString &workspaceTitle, const QJsonObject &page);
    void pageUpdated(const QString &workspaceTitle, const QJsonObject &page);
    void pageDeleted(const QString &workspaceTitle, const QString &title);

    // Элементы
    void elementAdded(const QString &workspaceTitle, const QString &pageTitle, const QJsonObject &element);
    void elementUpdated(const QString &workspaceTitle, const QString &pageTitle, const QJsonObject &element);
    void elementRemoved(const QString &workspaceTitle, const QString &pageTitle, const QString &elementId);

    // Синхронизация
    void syncCompleted(const QJsonObject &response);
    void syncError(const QString &error);

    // Общие
    void error(const QString &error);

private:
    QNetworkAccessManager *networkManager;
    QString baseUrl;
    QString authToken;
    QString _username;

    QNetworkRequest createRequest(const QString &endpoint);
    void handleResponse(QNetworkReply *reply, const std::function<void(const QJsonDocument &)> &successCallback);
};

#endif // API_CLIENT_H

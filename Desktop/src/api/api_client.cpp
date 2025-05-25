#include "api_client.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>
#include "../error_handler.h"

ApiClient::ApiClient(QObject *parent) :
    QObject(parent),
    networkManager(new QNetworkAccessManager(this)),
    baseUrl("http://localhost:8000/api")
{}

ApiClient::~ApiClient()
{
    delete networkManager;
}

void ApiClient::setBaseUrl(const QString &url)
{
    baseUrl = url;
}

void ApiClient::setAuthToken(const QString &token)
{
    authToken = token;
}

bool ApiClient::isAuthenticated() const
{
    return !authToken.isEmpty();
}

QString ApiClient::getUsername() const
{
    return _username;
}

QNetworkRequest ApiClient::createRequest(const QString &endpoint)
{
    QNetworkRequest request(QUrl(baseUrl + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!authToken.isEmpty()) {
        request.setRawHeader("Authorization", ("Token " + authToken).toUtf8());
    }
    return request;
}

void ApiClient::handleResponse(QNetworkReply *reply,
                               const std::function<void(const QJsonDocument &)> &successCallback)
{
    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            QJsonDocument jsonResponse = QJsonDocument::fromJson(response);
            successCallback(jsonResponse);
        } else {
            emit error(reply->errorString());
        }
        reply->deleteLater();
    });
}

// Аутентификация
void ApiClient::login(const QString &username, const QString &password)
{
    QJsonObject data;
    _username = username;
    data["username"] = username;
    data["password"] = password;

    QNetworkRequest request = createRequest("/auth/token/login/");
    QNetworkReply *reply = networkManager->post(request, QJsonDocument(data).toJson());

    handleResponse(reply, [this](const QJsonDocument &response) {
        if (response.isObject()) {
            QJsonObject obj = response.object();
            if (obj.contains("auth_token")) {
                authToken = obj["auth_token"].toString();
                emit loginSuccess(authToken);
            }
        }
    });
}

void ApiClient::logout()
{
    QNetworkRequest request = createRequest("/auth/token/logout/");
    QNetworkReply *reply = networkManager->post(request, QByteArray());

    handleResponse(reply, [this](const QJsonDocument &) {
        authToken.clear();
        _username.clear();
        emit logoutSuccess();
    });
}

// Рабочие пространства
void ApiClient::getWorkspaces()
{
    QNetworkRequest request = createRequest("/workspaces/");
    QNetworkReply *reply = networkManager->get(request);

    handleResponse(reply, [this](const QJsonDocument &response) {
        if (response.isArray()) {
            emit workspacesReceived(response.array());
        }
    });
}

void ApiClient::createWorkspace(const QJsonObject &workspaceData)
{
    QNetworkRequest request = createRequest("/workspaces/");
    QNetworkReply *reply = networkManager->post(request, QJsonDocument(workspaceData).toJson());
    handleResponse(reply, [this](const QJsonDocument &response) {
        if (response.isObject()) {
            emit workspaceCreated(response.object());
        }
    });
}

void ApiClient::updateWorkspace(const QString &title, const QJsonObject &workspaceData)
{
    QNetworkRequest request = createRequest(QString("/workspaces/%1/").arg(title));
    QNetworkReply *reply = networkManager->put(request, QJsonDocument(workspaceData).toJson());
    handleResponse(reply, [this](const QJsonDocument &response) {
        if (response.isObject()) {
            emit workspaceUpdated(response.object());
        }
    });
}

void ApiClient::deleteWorkspace(const QString &title)
{
    QNetworkRequest request = createRequest(QString("/workspaces/%1/").arg(title));
    QNetworkReply *reply = networkManager->deleteResource(request);

    handleResponse(reply, [this, title](const QJsonDocument &) {
        emit workspaceDeleted(title);
    });
}

// Страницы
void ApiClient::getPages(const QString &workspaceTitle)
{
    QNetworkRequest request = createRequest(QString("/workspaces/%1/pages/").arg(workspaceTitle));
    QNetworkReply *reply = networkManager->get(request);

    handleResponse(reply, [this, workspaceTitle](const QJsonDocument &response) {
        if (response.isArray()) {
            emit pagesReceived(workspaceTitle, response.array());
        }
    });
}

void ApiClient::createPage(const QString &workspaceTitle, const QString &title, bool isMain)
{
    QJsonObject data;
    data["title"] = title;
    data["is_main"] = isMain;

    QNetworkRequest request = createRequest(QString("/workspaces/%1/pages/").arg(workspaceTitle));
    QNetworkReply *reply = networkManager->post(request, QJsonDocument(data).toJson());

    handleResponse(reply, [this, workspaceTitle](const QJsonDocument &response) {
        if (response.isObject()) {
            emit pageCreated(workspaceTitle, response.object());
        }
    });
}

void ApiClient::updatePage(const QString &workspaceTitle,
                           const QString &title,
                           const QString &newTitle,
                           bool isMain)
{
    QJsonObject data;
    data["title"] = newTitle;
    data["is_main"] = isMain;

    QNetworkRequest request =
     createRequest(QString("/workspaces/%1/pages/%2/").arg(workspaceTitle, title));
    QNetworkReply *reply = networkManager->put(request, QJsonDocument(data).toJson());

    handleResponse(reply, [this, workspaceTitle](const QJsonDocument &response) {
        if (response.isObject()) {
            emit pageUpdated(workspaceTitle, response.object());
        }
    });
}

void ApiClient::deletePage(const QString &workspaceTitle, const QString &title)
{
    QNetworkRequest request =
     createRequest(QString("/workspaces/%1/pages/%2/").arg(workspaceTitle, title));
    QNetworkReply *reply = networkManager->deleteResource(request);

    handleResponse(reply, [this, workspaceTitle, title](const QJsonDocument &) {
        emit pageDeleted(workspaceTitle, title);
    });
}

// Элементы страниц
void ApiClient::addElement(const QString &workspaceTitle,
                           const QString &pageTitle,
                           const QJsonObject &element)
{
    QNetworkRequest request =
     createRequest(QString("/workspaces/%1/pages/%2/add_element/").arg(workspaceTitle, pageTitle));
    QNetworkReply *reply = networkManager->post(request, QJsonDocument(element).toJson());

    handleResponse(reply, [this, workspaceTitle, pageTitle](const QJsonDocument &response) {
        if (response.isObject()) {
            emit elementAdded(workspaceTitle, pageTitle, response.object());
        }
    });
}

void ApiClient::updateElement(const QString &workspaceTitle,
                              const QString &pageTitle,
                              const QString &elementId,
                              const QJsonObject &element)
{
    QNetworkRequest request = createRequest(QString("/workspaces/%1/pages/%2/update-element/%3/")
                                             .arg(workspaceTitle, pageTitle, elementId));
    QNetworkReply *reply =
     networkManager->sendCustomRequest(request, "PATCH", QJsonDocument(element).toJson());
    handleResponse(reply, [this, workspaceTitle, pageTitle](const QJsonDocument &response) {
        if (response.isObject()) {
            emit elementUpdated(workspaceTitle, pageTitle, response.object());
        }
    });
}

void ApiClient::removeElement(const QString &workspaceTitle,
                              const QString &pageTitle,
                              const QString &elementId)
{
    QNetworkRequest request = createRequest(QString("/workspaces/%1/pages/%2/remove-element/%3/")
                                             .arg(workspaceTitle, pageTitle, elementId));
    QNetworkReply *reply = networkManager->deleteResource(request);

    handleResponse(reply, [this, workspaceTitle, pageTitle, elementId](const QJsonDocument &) {
        emit elementRemoved(workspaceTitle, pageTitle, elementId);
    });
}

// Синхронизация
void ApiClient::syncWorkspace(const QString &workspaceTitle, const QJsonObject &changes)
{
    QNetworkRequest request = createRequest("/sync/");
    QJsonObject data;
    data["workspace_title"] = workspaceTitle;
    data["changes"] = changes;

    QNetworkReply *reply = networkManager->post(request, QJsonDocument(data).toJson());

    handleResponse(reply, [this](const QJsonDocument &response) {
        if (response.isObject()) {
            emit syncCompleted(response.object());
        }
    });
}

void ApiClient::syncUserWorkspaces()
{
    QNetworkRequest request = createRequest("/user-sync/");
    QNetworkReply *reply = networkManager->post(request, QByteArray());

    handleResponse(reply, [this](const QJsonDocument &response) {
        if (response.isObject()) {
            emit syncCompleted(response.object());
        }
    });
}

// Пользователь
void ApiClient::getCurrentUser()
{
    QNetworkRequest request = createRequest("/users/me/");
    QNetworkReply *reply = networkManager->get(request);

    handleResponse(reply, [this](const QJsonDocument &response) {
        if (response.isObject()) {
            QJsonObject user = response.object();
            _username = user["username"].toString();
        }
    });
}

void ApiClient::updateUser(const QJsonObject &userData)
{
    QNetworkRequest request = createRequest("/users/me/");
    QNetworkReply *reply = networkManager->put(request, QJsonDocument(userData).toJson());

    handleResponse(reply, [this](const QJsonDocument &response) {
        if (response.isObject()) {
            QJsonObject user = response.object();
            _username = user["username"].toString();
        }
    });
}

void ApiClient::deleteUser()
{
    QNetworkRequest request = createRequest("/users/me/");
    QNetworkReply *reply = networkManager->deleteResource(request);

    handleResponse(reply, [this](const QJsonDocument &) {
        authToken.clear();
        _username.clear();
    });
}

void ApiClient::registerUser(const QString &email, const QString &password, const QString &username)
{
    QJsonObject data;
    data["email"] = email;
    data["password"] = password;
    data["username"] = username;

    QNetworkRequest request = createRequest("/users/");
    QNetworkReply *reply = networkManager->post(request, QJsonDocument(data).toJson());

    handleResponse(reply, [this](const QJsonDocument &response) {
        if (response.isObject()) {
            emit registerSuccess();
        } else {
            emit registerError("Registration failed");
        }
    });
}

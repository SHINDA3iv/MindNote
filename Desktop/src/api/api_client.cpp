#include "api_client.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QUrlQuery>

ApiClient::ApiClient(QObject *parent)
    : QObject(parent)
    , networkManager(new QNetworkAccessManager(this))
    , baseUrl("http://localhost:8000/api/")
{
}

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

QNetworkRequest ApiClient::createRequest(const QString &endpoint)
{
    QNetworkRequest request(QUrl(baseUrl + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    if (!authToken.isEmpty()) {
        request.setRawHeader("Authorization", "Token " + authToken.toUtf8());
    }
    
    return request;
}

void ApiClient::handleResponse(QNetworkReply *reply, const std::function<void(const QJsonDocument &)> &successCallback)
{
    connect(reply, &QNetworkReply::finished, this, [=]() {
        reply->deleteLater();
        
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            QJsonDocument jsonResponse = QJsonDocument::fromJson(response);
            successCallback(jsonResponse);
        } else {
            emit error(reply->errorString());
        }
    });
}

void ApiClient::login(const QString &username, const QString &password)
{
    QJsonObject data;
    data["username"] = username;
    data["password"] = password;

    QNetworkRequest request = createRequest("auth/token/login/");
    QNetworkReply *reply = networkManager->post(request, QJsonDocument(data).toJson());

    handleResponse(reply, [this](const QJsonDocument &response) {
        if (response.isObject() && response.object().contains("auth_token")) {
            QString token = response.object()["auth_token"].toString();
            setAuthToken(token);
            emit loginSuccess(token);
        } else {
            emit loginError("Invalid response format");
        }
    });
}

void ApiClient::registerUser(const QString &username, const QString &email, const QString &password)
{
    QJsonObject data;
    data["username"] = username;
    data["email"] = email;
    data["password"] = password;

    QNetworkRequest request = createRequest("auth/users/");
    QNetworkReply *reply = networkManager->post(request, QJsonDocument(data).toJson());

    handleResponse(reply, [this](const QJsonDocument &response) {
        if (response.isObject()) {
            emit registerSuccess();
        } else {
            emit registerError("Invalid response format");
        }
    });
}

void ApiClient::getWorkspaces()
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }

    QNetworkRequest request = createRequest("workspaces/");
    QNetworkReply *reply = networkManager->get(request);

    handleResponse(reply, [this](const QJsonDocument &response) {
        if (response.isArray()) {
            emit workspacesReceived(response.array());
        } else {
            emit error("Invalid response format");
        }
    });
}

void ApiClient::createWorkspace(const QString &name, const QString &description)
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }

    QJsonObject data;
    data["name"] = name;
    data["description"] = description;

    QNetworkRequest request = createRequest("workspaces/");
    QNetworkReply *reply = networkManager->post(request, QJsonDocument(data).toJson());

    handleResponse(reply, [this](const QJsonDocument &response) {
        if (response.isObject()) {
            emit workspaceCreated(response.object());
        } else {
            emit error("Invalid response format");
        }
    });
}

void ApiClient::updateWorkspace(int id, const QString &name, const QString &description)
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }

    QJsonObject data;
    data["name"] = name;
    data["description"] = description;

    QNetworkRequest request = createRequest(QString("workspaces/%1/").arg(id));
    QNetworkReply *reply = networkManager->put(request, QJsonDocument(data).toJson());

    handleResponse(reply, [this](const QJsonDocument &response) {
        if (response.isObject()) {
            emit workspaceUpdated(response.object());
        } else {
            emit error("Invalid response format");
        }
    });
}

void ApiClient::deleteWorkspace(int id)
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }

    QNetworkRequest request = createRequest(QString("workspaces/%1/").arg(id));
    QNetworkReply *reply = networkManager->deleteResource(request);

    handleResponse(reply, [this, id](const QJsonDocument &) {
        emit workspaceDeleted(id);
    });
}

void ApiClient::getCurrentUser()
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }

    QNetworkRequest request = createRequest("users/me/");
    QNetworkReply *reply = networkManager->get(request);

    handleResponse(reply, [this](const QJsonDocument &response) {
        if (response.isObject()) {
            // Обработка данных пользователя
        } else {
            emit error("Invalid response format");
        }
    });
}

void ApiClient::updateUser(const QJsonObject &userData)
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }

    QNetworkRequest request = createRequest("users/me/");
    QNetworkReply *reply = networkManager->put(request, QJsonDocument(userData).toJson());

    handleResponse(reply, [this](const QJsonDocument &response) {
        if (response.isObject()) {
            // Обработка обновленных данных пользователя
        } else {
            emit error("Invalid response format");
        }
    });
}

void ApiClient::deleteUser()
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }

    QNetworkRequest request = createRequest("users/me/");
    QNetworkReply *reply = networkManager->deleteResource(request);

    handleResponse(reply, [this](const QJsonDocument &) {
        // Обработка удаления пользователя
    });
}

void ApiClient::getWorkspaceItems(const QString &workspaceId)
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }

    QNetworkRequest request = createRequest(QString("workspaces/%1/items/").arg(workspaceId));
    QNetworkReply *reply = networkManager->get(request);

    handleResponse(reply, [this, workspaceId](const QJsonDocument &response) {
        if (response.isArray()) {
            emit itemsReceived(workspaceId, response.array());
        } else {
            emit error("Invalid response format");
        }
    });
}

void ApiClient::createWorkspaceItem(const QString &workspaceId, const QJsonObject &data)
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }

    QNetworkRequest request = createRequest(QString("workspaces/%1/items/").arg(workspaceId));
    QNetworkReply *reply = networkManager->post(request, QJsonDocument(data).toJson());

    handleResponse(reply, [this, workspaceId](const QJsonDocument &response) {
        if (response.isObject()) {
            emit itemCreated(workspaceId, response.object());
        } else {
            emit error("Invalid response format");
        }
    });
}

void ApiClient::updateWorkspaceItem(const QString &workspaceId, const QString &itemId, const QJsonObject &data)
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }

    QNetworkRequest request = createRequest(QString("workspaces/%1/items/%2/").arg(workspaceId, itemId));
    QNetworkReply *reply = networkManager->put(request, QJsonDocument(data).toJson());

    handleResponse(reply, [this, workspaceId](const QJsonDocument &response) {
        if (response.isObject()) {
            emit itemUpdated(workspaceId, response.object());
        } else {
            emit error("Invalid response format");
        }
    });
}

void ApiClient::deleteWorkspaceItem(const QString &workspaceId, const QString &itemId)
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }

    QNetworkRequest request = createRequest(QString("workspaces/%1/items/%2/").arg(workspaceId, itemId));
    QNetworkReply *reply = networkManager->deleteResource(request);

    handleResponse(reply, [this, workspaceId, itemId](const QJsonDocument &) {
        emit itemDeleted(workspaceId, itemId);
    });
}

void ApiClient::syncChanges(const QJsonObject &changes)
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }

    QNetworkRequest request = createRequest("sync/");
    QNetworkReply *reply = networkManager->post(request, QJsonDocument(changes).toJson());

    handleResponse(reply, [this](const QJsonDocument &response) {
        if (response.isObject()) {
            emit syncCompleted(response.object());
        } else {
            emit error("Invalid response format");
        }
    });
}

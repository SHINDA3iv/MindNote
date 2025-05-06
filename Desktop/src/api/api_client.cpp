#include "api_client.h"
#include <qjsonarray.h>
#include <qjsonobject.h>
#include <QJsonDocument>
#include <QUrlQuery>

ApiClient::ApiClient(QObject *parent) : QObject(parent)
{
    _manager = std::make_unique<QNetworkAccessManager>(this);
    connect(_manager.get(), &QNetworkAccessManager::finished, this, &ApiClient::handleReply);
}

void ApiClient::login(const QString &email, const QString &password)
{
    QNetworkRequest request(QUrl(_baseUrl + "auth/login"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject data { { "email", email }, { "password", password } };

    auto reply = _manager->post(request, QJsonDocument(data).toJson());
    _pendingRequests[reply] = "login";
}

void ApiClient::registerUser(const QString &email, const QString &password, const QString &username)
{
    QNetworkRequest request(QUrl(_baseUrl + "auth/register"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject data { { "email", email }, { "password", password }, { "username", username } };

    auto reply = _manager->post(request, QJsonDocument(data).toJson());
    _pendingRequests[reply] = "register";
}

void ApiClient::getCurrentUser()
{
    if (!isAuthenticated()) {
        emit errorOccurred("Not authenticated");
        return;
    }

    QNetworkRequest request(QUrl(_baseUrl + "users/me"));
    request.setRawHeader("Authorization", ("Bearer " + _authToken).toUtf8());
    auto reply = _manager->get(request);
    _pendingRequests[reply] = "getCurrentUser";
}

void ApiClient::updateUser(const QJsonObject &userData)
{
    if (!isAuthenticated()) {
        emit errorOccurred("Not authenticated");
        return;
    }

    QNetworkRequest request(QUrl(_baseUrl + "user/me"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + _authToken).toUtf8());
    auto reply = _manager->put(request, QJsonDocument(userData).toJson());
    _pendingRequests[reply] = "updateUser";
}

void ApiClient::deleteUser()
{
    if (!isAuthenticated()) {
        emit errorOccurred("Not authenticated");
        return;
    }

    QNetworkRequest request(QUrl(_baseUrl + "user/me"));
    request.setRawHeader("Authorization", ("Bearer " + _authToken).toUtf8());
    auto reply = _manager->deleteResource(request);
    _pendingRequests[reply] = "deleteUser";
}

void ApiClient::getWorkspaces()
{
    if (!isAuthenticated()) {
        emit errorOccurred("Not authenticated");
        return;
    }

    QNetworkRequest request(QUrl(_baseUrl + "/api/workspaces"));
    request.setRawHeader("Authorization", ("Bearer " + _authToken).toUtf8());
    auto reply = _manager->get(request);
    _pendingRequests[reply] = "fetchWorkspaces";
}

void ApiClient::createWorkspace(const QJsonObject &data)
{
    if (!isAuthenticated()) {
        emit errorOccurred("Not authenticated");
        return;
    }

    QNetworkRequest request(QUrl(_baseUrl + "workspaces"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + _authToken).toUtf8());
    auto reply = _manager->post(request, QJsonDocument(data).toJson());
    _pendingRequests[reply] = "createWorkspace";
}

void ApiClient::updateWorkspace(int id, const QJsonObject &data)
{
    if (!isAuthenticated()) {
        emit errorOccurred("Not authenticated");
        return;
    }

    QNetworkRequest request(QUrl(_baseUrl + "/api/workspaces/" + QString::number(id)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + _authToken).toUtf8());
    auto reply = _manager->put(request, QJsonDocument(data).toJson());
    _pendingRequests[reply] = "updateWorkspace";
}

void ApiClient::deleteWorkspace(int id)
{
    if (!isAuthenticated()) {
        emit errorOccurred("Not authenticated");
        return;
    }

    QNetworkRequest request(QUrl(_baseUrl + "/api/workspaces/" + QString::number(id)));
    request.setRawHeader("Authorization", ("Bearer " + _authToken).toUtf8());
    auto reply = _manager->deleteResource(request);
    _pendingRequests[reply] = "deleteWorkspace";
}

void ApiClient::getWorkspaceItems(const QString &workspaceId)
{
    if (!isAuthenticated()) {
        emit errorOccurred("Not authenticated");
        return;
    }

    QNetworkRequest request(QUrl(_baseUrl + "workspaces/" + workspaceId + "/items"));
    request.setRawHeader("Authorization", ("Bearer " + _authToken).toUtf8());
    auto reply = _manager->get(request);
    _pendingRequests[reply] = "fetchWorkspaceItems";
}

void ApiClient::createWorkspaceItem(const QString &workspaceId, const QJsonObject &data)
{
    if (!isAuthenticated()) {
        emit errorOccurred("Not authenticated");
        return;
    }

    QNetworkRequest request(QUrl(_baseUrl + "/api/workspaces/" + workspaceId + "/items"));
    request.setRawHeader("Authorization", ("Bearer " + _authToken).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    auto reply = _manager->post(request, QJsonDocument(data).toJson());
    _pendingRequests[reply] = "createWorkspaceItem";
}

void ApiClient::updateWorkspaceItem(const QString &workspaceId,
                                    const QString &itemId,
                                    const QJsonObject &data)
{
    if (!isAuthenticated()) {
        emit errorOccurred("Not authenticated");
        return;
    }

    QNetworkRequest request(QUrl(_baseUrl + "/api/workspaces/" + workspaceId + "/items/" + itemId));
    request.setRawHeader("Authorization", ("Bearer " + _authToken).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    auto reply = _manager->put(request, QJsonDocument(data).toJson());
    _pendingRequests[reply] = "updateWorkspaceItem";
}

void ApiClient::deleteWorkspaceItem(const QString &workspaceId, const QString &itemId)
{
    if (!isAuthenticated()) {
        emit errorOccurred("Not authenticated");
        return;
    }

    QNetworkRequest request(QUrl(_baseUrl + "/api/workspaces/" + workspaceId + "/items/" + itemId));
    request.setRawHeader("Authorization", ("Bearer " + _authToken).toUtf8());
    auto reply = _manager->deleteResource(request);
    _pendingRequests[reply] = "deleteWorkspaceItem";
}

void ApiClient::setAuthToken(const QString &token)
{
    _authToken = token;
}

bool ApiClient::isAuthenticated() const
{
    return !_authToken.isEmpty();
}

void ApiClient::handleReply(QNetworkReply *reply)
{
    if (!_pendingRequests.contains(reply))
        return;

    QString requestType = _pendingRequests.take(reply);
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray response = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(response);
    if (doc.isNull()) {
        emit errorOccurred("Invalid JSON response");
        reply->deleteLater();
        return;
    }

    if (requestType == "fetchWorkspaces") {
        emit workspacesReceived(doc.array());
    } else if (requestType == "createWorkspace") {
        emit workspaceCreated(doc.object());
    } else if (requestType == "updateWorkspace") {
        emit workspaceUpdated(doc.object());
    } else if (requestType == "deleteWorkspace") {
        emit workspaceDeleted(doc.object()["id"].toString());
    } else if (requestType == "fetchWorkspaceItems") {
        QString workspaceId = reply->url().path().split('/')[3];
        emit itemsReceived(workspaceId, doc.array());
    } else if (requestType == "createWorkspaceItem") {
        QString workspaceId = reply->url().path().split('/')[3];
        emit itemCreated(workspaceId, doc.object());
    } else if (requestType == "updateWorkspaceItem") {
        QString workspaceId = reply->url().path().split('/')[3];
        emit itemUpdated(workspaceId, doc.object());
    } else if (requestType == "deleteWorkspaceItem") {
        QString workspaceId = reply->url().path().split('/')[3];
        QString itemId = reply->url().path().split('/')[5];
        emit itemDeleted(workspaceId, itemId);
    } else if (requestType == "sync") {
        emit syncCompleted(doc.object());
    } else if (requestType == "login") {
        if (doc.object().contains("token")) {
            _authToken = doc.object()["token"].toString();
            emit loginSuccess(_authToken);
        } else {
            emit loginFailed(doc.object()["error"].toString());
        }
    } else if (requestType == "register") {
        if (reply->error() == QNetworkReply::NoError) {
            emit registrationSuccess();
        } else {
            emit registrationFailed(doc.object()["error"].toString());
        }
    }

    reply->deleteLater();
}

void ApiClient::syncChanges(const QJsonObject &localChanges)
{
    if (!isAuthenticated()) {
        emit errorOccurred("Not authenticated");
        return;
    }

    QNetworkRequest request(QUrl(_baseUrl + "sync/"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + _authToken).toUtf8());
    QNetworkReply *reply = _manager->post(request, QJsonDocument(localChanges).toJson());
    _pendingRequests[reply] = "sync";
}

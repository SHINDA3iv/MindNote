#include "api_client.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QUrlQuery>
#include "../error_handler.h"

ApiClient::ApiClient(QObject *parent) :
    QObject(parent),
    networkManager(new QNetworkAccessManager(this)),
    baseUrl("http://localhost:8000/api/")
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
        request.setRawHeader("Authorization", "Token " + authToken.toUtf8());
    }

    return request;
}

void ApiClient::handleResponse(QNetworkReply *reply,
                               const std::function<void(const QJsonDocument &)> &successCallback)
{
    connect(reply, &QNetworkReply::finished, this, [=]() {
        reply->deleteLater();

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            QJsonDocument jsonResponse = QJsonDocument::fromJson(response);
            successCallback(jsonResponse);
        } else {
            QByteArray response = reply->readAll();
            QJsonDocument jsonResponse = QJsonDocument::fromJson(response);
            int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

            if (statusCode == 400 && jsonResponse.isObject()) {
                QString errorMsg;
                QJsonObject errorObj = jsonResponse.object();
                
                // Обработка специальных случаев ошибок
                if (errorObj.contains("non_field_errors")) {
                    QJsonValue nonFieldErrors = errorObj["non_field_errors"];
                    if (nonFieldErrors.isArray()) {
                        QString error = nonFieldErrors.toArray().first().toString();
                        if (error.contains("Unable to log in with provided credentials")) {
                            errorMsg = "Неверное имя пользователя или пароль";
                        } else if (error.contains("Invalid token")) {
                            errorMsg = "Недействительный токен авторизации";
                        } else if (error.contains("Token has expired")) {
                            errorMsg = "Срок действия токена истек. Пожалуйста, войдите снова";
                        } else if (error.contains("User is inactive")) {
                            errorMsg = "Аккаунт деактивирован";
                        } else if (error.contains("User account is disabled")) {
                            errorMsg = "Аккаунт заблокирован";
                        } else {
                            errorMsg = error;
                        }
                    }
                } else {
                    // Обработка ошибок валидации полей
                    for (const QString &key : errorObj.keys()) {
                        QString fieldName;
                        // Преобразование имен полей в более понятные
                        if (key == "username") fieldName = "Имя пользователя";
                        else if (key == "password") fieldName = "Пароль";
                        else if (key == "email") fieldName = "Email";
                        else if (key == "name") fieldName = "Название";
                        else if (key == "description") fieldName = "Описание";
                        else if (key == "token") fieldName = "Токен авторизации";
                        else fieldName = key;

                        QJsonValue value = errorObj.value(key);
                        if (value.isArray()) {
                            QStringList errors;
                            for (const QJsonValue &val : value.toArray()) {
                                QString error = val.toString();
                                // Преобразование сообщений об ошибках
                                if (error.contains("already exists")) {
                                    if (key == "username") {
                                        error = "Пользователь с таким именем уже существует";
                                    } else if (key == "email") {
                                        error = "Пользователь с таким email уже существует";
                                    }
                                } else if (error.contains("required")) {
                                    error = "Это поле обязательно для заполнения";
                                } else if (error.contains("invalid")) {
                                    if (key == "password") {
                                        error = "Пароль должен содержать минимум 8 символов, включая буквы и цифры";
                                    } else if (key == "email") {
                                        error = "Неверный формат email адреса";
                                    } else if (key == "token") {
                                        error = "Неверный формат токена авторизации";
                                    } else {
                                        error = "Неверный формат";
                                    }
                                } else if (error.contains("too short")) {
                                    if (key == "password") {
                                        error = "Пароль слишком короткий (минимум 8 символов)";
                                    } else if (key == "username") {
                                        error = "Имя пользователя слишком короткое (минимум 3 символа)";
                                    }
                                } else if (error.contains("too long")) {
                                    if (key == "username") {
                                        error = "Имя пользователя слишком длинное (максимум 150 символов)";
                                    } else if (key == "password") {
                                        error = "Пароль слишком длинный (максимум 128 символов)";
                                    }
                                } else if (error.contains("not found")) {
                                    if (key == "username") {
                                        error = "Пользователь не найден";
                                    }
                                } else if (error.contains("incorrect")) {
                                    if (key == "password") {
                                        error = "Неверный пароль";
                                    }
                                } else if (error.contains("too similar")) {
                                    if (key == "password") {
                                        error = "Пароль слишком похож на имя пользователя. Используйте более сложный пароль";
                                    }
                                } else if (error.contains("Enter a valid email address")) {
                                    if (key == "email") {
                                        error = "Введите корректный email адрес (например: user@example.com)";
                                    }
                                }
                                errors.append(error);
                            }
                            errorMsg += fieldName + ": " + errors.join(", ") + "\n";
                        } else {
                            errorMsg += fieldName + ": " + value.toString() + "\n";
                        }
                    }
                }

                // Удаляем последний перенос строки
                if (!errorMsg.isEmpty()) {
                    errorMsg.chop(1);
                }

                ErrorHandler::instance().showError("Ошибка", errorMsg);
                emit error(errorMsg);
            } else if (statusCode == 401) {
                QString errorMsg = "Требуется авторизация. Пожалуйста, войдите в систему.";
                ErrorHandler::instance().showError("Ошибка авторизации", errorMsg);
                emit error(errorMsg);
            } else if (statusCode == 403) {
                QString errorMsg = "Доступ запрещен. У вас нет прав для выполнения этого действия.";
                ErrorHandler::instance().showError("Ошибка доступа", errorMsg);
                emit error(errorMsg);
            } else {
                QString errorMsg = reply->errorString();
                // Преобразование сетевых ошибок
                if (errorMsg.contains("Connection refused")) {
                    errorMsg = "Не удалось подключиться к серверу. Проверьте подключение к интернету.";
                } else if (errorMsg.contains("timeout")) {
                    errorMsg = "Превышено время ожидания ответа от сервера.";
                } else if (errorMsg.contains("SSL handshake failed")) {
                    errorMsg = "Ошибка безопасного соединения с сервером.";
                }
                
                ErrorHandler::instance().showError("Ошибка", errorMsg);
                emit error(errorMsg);
            }
        }
    });
}

void ApiClient::login(const QString &username, const QString &password)
{
    QJsonObject data;
    data["username"] = username;
    data["password"] = password;
    _username = username; // Store username

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

void ApiClient::registerUser(const QString &email, const QString &password, const QString &username)
{
    if (username.isEmpty() || password.isEmpty()) {
        emit registerError("Username and password are required");
        return;
    }

    QJsonObject data;
    data["username"] = username;
    data["password"] = password;
    if (!email.isEmpty()) {
        data["email"] = email;
    }

    QNetworkRequest request = createRequest("users/");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = networkManager->post(request, QJsonDocument(data).toJson());

    handleResponse(reply, [this](const QJsonDocument &response) {
        if (response.isObject() && response.object().contains("username")) {
            emit registerSuccess();
        } else {
            // Generic error handling is now in handleResponse,
            // but we can add specific checks if needed later
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

    QNetworkRequest request = createRequest(QString("workspaces/%1").arg(id));
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

    QNetworkRequest request = createRequest(QString("workspaces/%1").arg(id));
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

void ApiClient::validateToken()
{
    if (!isAuthenticated()) {
        emit tokenInvalid();
        return;
    }

    QNetworkRequest request = createRequest("users/me/");
    QNetworkReply *reply = networkManager->get(request);

    handleResponse(reply, [this](const QJsonDocument &response) {
        if (response.isObject() && response.object().contains("username")) {
            emit tokenValid();
        } else {
            emit tokenInvalid();
        }
    });
}

void ApiClient::getWorkspaceItems(const QString &workspaceId)
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }

    // First get the main page of the workspace
    QNetworkRequest request = createRequest(QString("workspaces/%1").arg(workspaceId));
    QNetworkReply *reply = networkManager->get(request);

    handleResponse(reply, [this, workspaceId](const QJsonDocument &response) {
        if (response.isObject()) {
            QJsonObject workspace = response.object();
            if (workspace.contains("pages") && workspace["pages"].isArray()) {
                QJsonArray pages = workspace["pages"].toArray();
                // Get items from the main page
                for (const QJsonValue &pageValue : pages) {
                    QJsonObject page = pageValue.toObject();
                    if (page["is_main"].toBool()) {
                        emit itemsReceived(workspaceId, page["elements"].toArray());
                        break;
                    }
                }
            } else {
                emit error("Invalid response format");
            }
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

    // First get the main page ID
    QNetworkRequest request = createRequest(QString("workspaces/%1/").arg(workspaceId));
    QNetworkReply *reply = networkManager->get(request);

    handleResponse(reply, [this, workspaceId, data](const QJsonDocument &response) {
        if (response.isObject()) {
            QJsonObject workspace = response.object();
            if (workspace.contains("pages") && workspace["pages"].isArray()) {
                QJsonArray pages = workspace["pages"].toArray();
                // Find the main page
                for (const QJsonValue &pageValue : pages) {
                    QJsonObject page = pageValue.toObject();
                    if (page["is_main"].toBool()) {
                        QString pageId = page["id"].toString();
                        // Add element to the main page
                        QNetworkRequest addRequest = createRequest(
                         QString("workspaces/%1/pages/%2/elements/").arg(workspaceId, pageId));
                        QNetworkReply *addReply =
                         networkManager->post(addRequest, QJsonDocument(data).toJson());

                        handleResponse(addReply,
                                       [this, workspaceId](const QJsonDocument &response) {
                                           if (response.isObject()) {
                                               emit itemCreated(workspaceId, response.object());
                                           } else {
                                               emit error("Invalid response format");
                                           }
                                       });
                        break;
                    }
                }
            } else {
                emit error("Invalid response format");
            }
        } else {
            emit error("Invalid response format");
        }
    });
}

void ApiClient::updateWorkspaceItem(const QString &workspaceId,
                                    const QString &itemId,
                                    const QJsonObject &data)
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }

    // First get the main page ID
    QNetworkRequest request = createRequest(QString("workspaces/%1/").arg(workspaceId));
    QNetworkReply *reply = networkManager->get(request);

    handleResponse(reply, [this, workspaceId, itemId, data](const QJsonDocument &response) {
        if (response.isObject()) {
            QJsonObject workspace = response.object();
            if (workspace.contains("pages") && workspace["pages"].isArray()) {
                QJsonArray pages = workspace["pages"].toArray();
                // Find the main page
                for (const QJsonValue &pageValue : pages) {
                    QJsonObject page = pageValue.toObject();
                    if (page["is_main"].toBool()) {
                        QString pageId = page["id"].toString();
                        // Update element in the main page
                        QNetworkRequest updateRequest =
                         createRequest(QString("workspaces/%1/pages/%2/elements/%3/")
                                        .arg(workspaceId, pageId, itemId));
                        QNetworkReply *updateReply = networkManager->sendCustomRequest(
                         updateRequest, "PATCH", QJsonDocument(data).toJson());

                        handleResponse(updateReply,
                                       [this, workspaceId](const QJsonDocument &response) {
                                           if (response.isObject()) {
                                               emit itemUpdated(workspaceId, response.object());
                                           } else {
                                               emit error("Invalid response format");
                                           }
                                       });
                        break;
                    }
                }
            } else {
                emit error("Invalid response format");
            }
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

    // First get the main page ID
    QNetworkRequest request = createRequest(QString("workspaces/%1/").arg(workspaceId));
    QNetworkReply *reply = networkManager->get(request);

    handleResponse(reply, [this, workspaceId, itemId](const QJsonDocument &response) {
        if (response.isObject()) {
            QJsonObject workspace = response.object();
            if (workspace.contains("pages") && workspace["pages"].isArray()) {
                QJsonArray pages = workspace["pages"].toArray();
                // Find the main page
                for (const QJsonValue &pageValue : pages) {
                    QJsonObject page = pageValue.toObject();
                    if (page["is_main"].toBool()) {
                        QString pageId = page["id"].toString();
                        // Remove element from the main page
                        QNetworkRequest deleteRequest =
                         createRequest(QString("workspaces/%1/pages/%2/elements/%3/")
                                        .arg(workspaceId, pageId, itemId));
                        QNetworkReply *deleteReply = networkManager->deleteResource(deleteRequest);

                        handleResponse(deleteReply,
                                       [this, workspaceId, itemId](const QJsonDocument &) {
                                           emit itemDeleted(workspaceId, itemId);
                                       });
                        break;
                    }
                }
            } else {
                emit error("Invalid response format");
            }
        } else {
            emit error("Invalid response format");
        }
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

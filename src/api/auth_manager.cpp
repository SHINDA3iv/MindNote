// src/auth/auth_manager.cpp
#include "auth_manager.h"
#include <QCryptographicHash>

AuthManager::AuthManager(QObject *parent) : QObject(parent), _settings("MyCompany", "StudyProject")
{
    _authToken = _settings.value("auth/token").toString();
    _currentUserId = _settings.value("auth/userId").toString();
}

bool AuthManager::isAuthenticated() const
{
    return !_authToken.isEmpty();
}

QString AuthManager::getCurrentUserId() const
{
    return _currentUserId;
}

QString AuthManager::getAuthToken() const
{
    return _authToken;
}

void AuthManager::login(const QString &email, const QString &password)
{
    // Здесь должна быть логика отправки запроса на сервер
    // Временная реализация для демонстрации

    if (email.isEmpty() || password.isEmpty()) {
        emit loginFailed("Email and password cannot be empty");
        return;
    }

    // В реальном приложении здесь будет запрос к API
    _authToken =
     QCryptographicHash::hash((email + password).toUtf8(), QCryptographicHash::Sha256).toHex();
    _currentUserId = "user_" + email.split("@").first();

    _settings.setValue("auth/token", _authToken);
    _settings.setValue("auth/userId", _currentUserId);

    emit loginSuccess();
}

void AuthManager::registerUser(const QString &email,
                               const QString &password,
                               const QString &username)
{
    // Здесь должна быть логика отправки запроса на сервер
    // Временная реализация для демонстрации

    if (email.isEmpty() || password.isEmpty() || username.isEmpty()) {
        emit registrationFailed("All fields are required");
        return;
    }

    // В реальном приложении здесь будет запрос к API
    emit registrationSuccess();
}

void AuthManager::logout()
{
    _authToken.clear();
    _currentUserId.clear();
    _settings.remove("auth/token");
    _settings.remove("auth/userId");
    _settings.sync();
}

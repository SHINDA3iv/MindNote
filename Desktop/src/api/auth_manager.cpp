// src/auth/auth_manager.cpp
#include "auth_manager.h"
#include <QCryptographicHash>

AuthManager::AuthManager(QObject *parent) : QObject(parent), _settings("MyCompany", "Desktop")
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
    if (email.isEmpty() || password.isEmpty()) {
        emit loginFailed("Email and password cannot be empty");
        return;
    }

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
    if (email.isEmpty() || password.isEmpty() || username.isEmpty()) {
        emit registrationFailed("All fields are required");
        return;
    }

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

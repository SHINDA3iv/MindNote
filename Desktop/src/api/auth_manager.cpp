// src/auth/auth_manager.cpp
#include "auth_manager.h"
#include <QCryptographicHash>

AuthManager::AuthManager(QObject *parent)
    : QObject(parent)
    , _settings("StudyProject", "Auth")
    , _isAuthenticated(false)
{
    loadAuthState();
}

AuthManager::~AuthManager()
{
    saveAuthState();
}

bool AuthManager::isAuthenticated() const
{
    return _isAuthenticated;
}

QString AuthManager::getUsername() const
{
    return _username;
}

QString AuthManager::getAuthToken() const
{
    return _authToken;
}

void AuthManager::login(const QString &token, const QString &username)
{
    _authToken = token;
    _username = username;
    _isAuthenticated = true;
    saveAuthState();
    emit authStateChanged();
}

void AuthManager::logout()
{
    _authToken.clear();
    _username.clear();
    _isAuthenticated = false;
    saveAuthState();
    emit authStateChanged();
}

void AuthManager::saveAuthState()
{
    _settings.setValue("authToken", _authToken);
    _settings.setValue("username", _username);
    _settings.setValue("isAuthenticated", _isAuthenticated);
}

void AuthManager::loadAuthState()
{
    _authToken = _settings.value("authToken").toString();
    _username = _settings.value("username").toString();
    _isAuthenticated = _settings.value("isAuthenticated", false).toBool();
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

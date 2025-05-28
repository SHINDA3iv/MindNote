// src/auth/auth_manager.cpp
#include "auth_manager.h"
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>

AuthManager::AuthManager(QObject *parent)
    : QObject(parent)
    , _isAuthenticated(false)
    , _rememberMe(false)
{
    // Создаём папку settings, если не существует
    QString settingsDir = QCoreApplication::applicationDirPath() + "/settings";
    QDir().mkpath(settingsDir);
    QString authSettingsPath = settingsDir + "/Auth.ini";
    _settings = new QSettings(authSettingsPath, QSettings::IniFormat, this);
    loadAuthState();
}

AuthManager::~AuthManager()
{
    qDebug() << "delete";
    saveAuthState();
    delete _settings;
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

bool AuthManager::isRememberMeEnabled() const
{
    return _rememberMe;
}

void AuthManager::login(const QString &token, const QString &username, bool rememberMe)
{
    qDebug() << "Login called - token:" << token << "username:" << username << "rememberMe:" << rememberMe;
    _authToken = token;
    _username = username;
    _isAuthenticated = true;
    _rememberMe = rememberMe;
    qDebug() << "login";
    saveAuthState();
    emit authStateChanged();
}

void AuthManager::logout()
{
    _authToken.clear();
    _username.clear();
    _isAuthenticated = false;
    if (!_rememberMe) {
        _settings->remove("authToken");
        _settings->remove("username");
        _settings->remove("isAuthenticated");
        _settings->remove("rememberMe");
    }
    qDebug() << "LOGOUT";
    saveAuthState();
    emit authStateChanged();
}

void AuthManager::saveAuthState()
{
    qDebug() << "Saving auth state - rememberMe:" << _rememberMe;
    if (_rememberMe) {
        _settings->setValue("authToken", _authToken);
        _settings->setValue("username", _username);
        _settings->setValue("isAuthenticated", _isAuthenticated);
        _settings->setValue("rememberMe", _rememberMe);
        qDebug() << "Saved auth state - token:" << _authToken << "username:" << _username << "isAuthenticated:" << _isAuthenticated;
    } else {
        qDebug() << "Remember me is disabled, clearing saved auth state";
        _settings->remove("authToken");
        _settings->remove("username");
        _settings->remove("isAuthenticated");
        _settings->remove("rememberMe");
    }
}

void AuthManager::loadAuthState()
{
    _rememberMe = _settings->value("rememberMe", false).toBool();
    qDebug() << "Loading auth state - rememberMe:" << _rememberMe;
    
    if (_rememberMe) {
        _authToken = _settings->value("authToken").toString();
        _username = _settings->value("username").toString();
        _isAuthenticated = _settings->value("isAuthenticated", false).toBool();
        qDebug() << "Loaded auth state - token:" << _authToken << "username:" << _username << "isAuthenticated:" << _isAuthenticated;
    } else {
        qDebug() << "Remember me is disabled, clearing auth state";
        _authToken.clear();
        _username.clear();
        _isAuthenticated = false;
    }
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

void AuthManager::setRememberMe(bool enabled)
{
    _rememberMe = enabled;
    saveAuthState();
    qDebug() << "setRememberMe";
}

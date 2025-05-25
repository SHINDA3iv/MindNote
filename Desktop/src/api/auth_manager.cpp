// src/auth/auth_manager.cpp
#include "auth_manager.h"
#include <QCryptographicHash>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QRegularExpression>

AuthManager::AuthManager(QObject *parent) :
    QObject(parent),
    _settings("StudyProject", "Auth"),
    _isAuthenticated(false),
    _rememberMe(false)
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

bool AuthManager::isRememberMeEnabled() const
{
    return _rememberMe;
}

void AuthManager::login(const QString &token, const QString &username, bool rememberMe)
{
    qDebug() << "Login called - token:" << token << "username:" << username
             << "rememberMe:" << rememberMe;
    _authToken = token;
    _username = username;
    _isAuthenticated = true;
    _rememberMe = rememberMe;
    saveAuthState();
    emit authStateChanged();
}

void AuthManager::logout()
{
    _authToken.clear();
    _username.clear();
    _isAuthenticated = false;
    if (!_rememberMe) {
        _settings.remove("authToken");
        _settings.remove("username");
        _settings.remove("isAuthenticated");
        _settings.remove("rememberMe");
    }
    saveAuthState();
    emit authStateChanged();
}

void AuthManager::saveAuthState()
{
    qDebug() << "Saving auth state - rememberMe:" << _rememberMe;
    if (_rememberMe) {
        _settings.setValue("authToken", _authToken);
        _settings.setValue("username", _username);
        _settings.setValue("isAuthenticated", _isAuthenticated);
        _settings.setValue("rememberMe", _rememberMe);
        qDebug() << "Saved auth state - token:" << _authToken << "username:" << _username
                 << "isAuthenticated:" << _isAuthenticated;
    } else {
        qDebug() << "Remember me is disabled, clearing saved auth state";
        _settings.remove("authToken");
        _settings.remove("username");
        _settings.remove("isAuthenticated");
        _settings.remove("rememberMe");
    }
}

void AuthManager::loadAuthState()
{
    _rememberMe = _settings.value("rememberMe", false).toBool();
    qDebug() << "Loading auth state - rememberMe:" << _rememberMe;

    if (_rememberMe) {
        _authToken = _settings.value("authToken").toString();
        _username = _settings.value("username").toString();
        _isAuthenticated = _settings.value("isAuthenticated", false).toBool();
        qDebug() << "Loaded auth state - token:" << _authToken << "username:" << _username
                 << "isAuthenticated:" << _isAuthenticated;
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
        emit registrationFailed("Все поля обязательны для заполнения");
        return;
    }

    // Проверяем формат email
    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    if (!emailRegex.match(email).hasMatch()) {
        emit registrationFailed("Неверный формат email");
        return;
    }

    // Проверяем длину пароля
    if (password.length() < 8) {
        emit registrationFailed("Пароль должен содержать минимум 8 символов");
        return;
    }

    // Проверяем длину имени пользователя
    if (username.length() < 3) {
        emit registrationFailed("Имя пользователя должно содержать минимум 3 символа");
        return;
    }

    // Хешируем пароль
    QByteArray passwordHash =
     QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();

    // TODO: Отправить запрос на регистрацию на сервер
    // Здесь должен быть код для отправки данных на сервер
    // и обработки ответа

    // Временная заглушка для тестирования
    emit registrationSuccess();
}

void AuthManager::setRememberMe(bool enabled)
{
    _rememberMe = enabled;
    saveAuthState();
}

bool AuthManager::validateToken() const
{
    if (!_isAuthenticated || _authToken.isEmpty()) {
        return false;
    }

    // Проверяем формат токена (должен быть JWT)
    if (!_authToken.contains('.')) {
        return false;
    }

    // Разбиваем токен на части
    QStringList parts = _authToken.split('.');
    if (parts.size() != 3) {
        return false;
    }

    // Проверяем срок действия токена
    QByteArray payload = QByteArray::fromBase64(parts[1].toUtf8());
    QJsonDocument doc = QJsonDocument::fromJson(payload);
    if (doc.isNull()) {
        return false;
    }

    QJsonObject obj = doc.object();
    if (!obj.contains("exp")) {
        return false;
    }

    qint64 exp = obj["exp"].toDouble();
    qint64 currentTime = QDateTime::currentSecsSinceEpoch();

    return exp > currentTime;
}

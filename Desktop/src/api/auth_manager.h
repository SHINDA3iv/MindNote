// src/auth/auth_manager.h
#ifndef AUTH_MANAGER_H
#define AUTH_MANAGER_H

#include <QObject>
#include <QSettings>
#include <QString>

class AuthManager : public QObject
{
    Q_OBJECT
public:
    explicit AuthManager(QObject *parent = nullptr);
    ~AuthManager();

    bool isAuthenticated() const;
    QString getUsername() const;
    QString getAuthToken() const;
    bool isRememberMeEnabled() const;

    void login(const QString &token, const QString &username, bool rememberMe = false);
    void setRememberMe(bool enabled);
    void logout();
    void registerUser(const QString &email, const QString &password, const QString &username);

signals:
    void authStateChanged();
    void loginRequested();
    void logoutRequested();
    void registrationSuccess();
    void registrationFailed(const QString &error);

private:
    void saveAuthState();
    void loadAuthState();

    QString _authToken;
    QString _username;
    bool _isAuthenticated;
    bool _rememberMe;
    QSettings _settings;
};

#endif // AUTH_MANAGER_H

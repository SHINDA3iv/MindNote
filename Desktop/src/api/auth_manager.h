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

    bool isAuthenticated() const;
    QString getCurrentUserId() const;
    QString getAuthToken() const;

    void login(const QString &email, const QString &password);
    void registerUser(const QString &email, const QString &password, const QString &username);
    void logout();

signals:
    void loginSuccess();
    void loginFailed(const QString &error);
    void registrationSuccess();
    void registrationFailed(const QString &error);

private:
    QSettings _settings;
    QString _authToken;
    QString _currentUserId;
};

#endif // AUTH_MANAGER_H

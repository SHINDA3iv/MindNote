// src/view/auth_dialog.h
#ifndef AUTH_DIALOG_H
#define AUTH_DIALOG_H

#include <QDialog>
#include <QCloseEvent>
#include <qtabwidget.h>

class QLineEdit;
class QPushButton;
class QTabWidget;
class QCheckBox;
class QLabel;

class AuthDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AuthDialog(QWidget *parent = nullptr);

    QTabWidget *tabWidget() const
    {
        return _tabWidget;
    }
    void setCurrentTab(int index)
    {
        _tabWidget->setCurrentIndex(index);
    }

    void showLoginError(const QString &error);
    void showRegisterError(const QString &error);

signals:
    void loginRequested(const QString &username, const QString &password, bool rememberMe);
    void registerRequested(const QString &email, const QString &password, const QString &username);
    void guestLoginRequested();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    QTabWidget *_tabWidget;
    QLineEdit *_loginUsername;
    QLineEdit *_loginPassword;
    QPushButton *_loginButton;
    QCheckBox *_rememberMe;
    QLabel *_loginErrorLabel;

    QLineEdit *_registerEmail;
    QLineEdit *_registerPassword;
    QLineEdit *_registerUsername;
    QPushButton *_registerButton;
    QLabel *_registerErrorLabel;
};

#endif // AUTH_DIALOG_H

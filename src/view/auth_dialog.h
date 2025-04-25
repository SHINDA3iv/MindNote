// src/view/auth_dialog.h
#ifndef AUTH_DIALOG_H
#define AUTH_DIALOG_H

#include <QDialog>
#include <qtabwidget.h>

class QLineEdit;
class QPushButton;
class QTabWidget;

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

signals:
    void loginRequested(const QString &email, const QString &password);
    void registerRequested(const QString &email, const QString &password, const QString &username);

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    QTabWidget *_tabWidget;
    QLineEdit *_loginEmail;
    QLineEdit *_loginPassword;
    QPushButton *_loginButton;

    QLineEdit *_registerEmail;
    QLineEdit *_registerPassword;
    QLineEdit *_registerUsername;
    QPushButton *_registerButton;
};

#endif // AUTH_DIALOG_H

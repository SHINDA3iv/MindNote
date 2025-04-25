// src/view/auth_dialog.cpp
#include "auth_dialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QLabel>

AuthDialog::AuthDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Authentication");
    setMinimumSize(300, 200);

    _tabWidget = new QTabWidget(this);

    // Login tab
    QWidget *loginTab = new QWidget;
    QFormLayout *loginLayout = new QFormLayout(loginTab);

    _loginEmail = new QLineEdit;
    _loginEmail->setPlaceholderText("user@example.com");
    loginLayout->addRow("Email:", _loginEmail);

    _loginPassword = new QLineEdit;
    _loginPassword->setPlaceholderText("Password");
    _loginPassword->setEchoMode(QLineEdit::Password);
    loginLayout->addRow("Password:", _loginPassword);

    _loginButton = new QPushButton("Login");
    loginLayout->addRow(_loginButton);

    connect(_loginButton, &QPushButton::clicked, this, &AuthDialog::onLoginClicked);

    // Register tab
    QWidget *registerTab = new QWidget;
    QFormLayout *registerLayout = new QFormLayout(registerTab);

    _registerEmail = new QLineEdit;
    _registerEmail->setPlaceholderText("user@example.com");
    registerLayout->addRow("Email:", _registerEmail);

    _registerUsername = new QLineEdit;
    _registerUsername->setPlaceholderText("Username");
    registerLayout->addRow("Username:", _registerUsername);

    _registerPassword = new QLineEdit;
    _registerPassword->setPlaceholderText("Password");
    _registerPassword->setEchoMode(QLineEdit::Password);
    registerLayout->addRow("Password:", _registerPassword);

    _registerButton = new QPushButton("Register");
    registerLayout->addRow(_registerButton);

    connect(_registerButton, &QPushButton::clicked, this, &AuthDialog::onRegisterClicked);

    _tabWidget->addTab(loginTab, "Login");
    _tabWidget->addTab(registerTab, "Register");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(_tabWidget);
}

void AuthDialog::onLoginClicked()
{
    emit loginRequested(_loginEmail->text(), _loginPassword->text());
}

void AuthDialog::onRegisterClicked()
{
    emit registerRequested(_registerEmail->text(), _registerPassword->text(),
                           _registerUsername->text());
}

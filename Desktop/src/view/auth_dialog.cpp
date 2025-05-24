// src/view/auth_dialog.cpp
#include "auth_dialog.h"
#include <QFormLayout>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpacerItem>
#include <QStyle>
#include <QTabWidget>
#include <QVBoxLayout>
#include <qevent.h>
#include <qtabbar.h>
#include <QCheckBox>

AuthDialog::AuthDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Авторизация");
    setMinimumSize(400, 350);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setAttribute(Qt::WA_QuitOnClose, true);

    // Главный лейаут
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // Настройка стилей для всего окна авторизации
    setStyleSheet(R"(
        QDialog {
            background-color: #f5f5f5;
        }
        QLabel#titleLabel {
            background-color: #f5f5f5;
            font-size: 18pt;
            font-weight: bold;
            qproperty-alignment: AlignCenter;
        }
        QTabWidget::pane {
            border: 1px solid #ddd;
            border-radius: 4px;
            margin-top: -1px;
        }
        QTabBar::tab {
            padding: 8px;
            border: 1px solid #ddd;
            background: #e0e0e0;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
        QTabBar::tab:selected {
            background: #f5f5f5;
            border-bottom-color: #f5f5f5;
        }
        QLineEdit {
            padding: 8px;
            border: 1px solid #ccc;
            border-radius: 4px;
        }
        QPushButton#loginButton {
            padding: 8px;
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 4px;
        }
        QPushButton#registerButton {
            padding: 8px;
            background-color: #2196F3;
            color: white;
            border: none;
            border-radius: 4px;
        }
        QPushButton#guestButton {
            padding: 8px;
            background-color: #9E9E9E;
            color: white;
            border: none;
            border-radius: 4px;
        }
        QFrame[frameShape="4"] { /* QFrame::HLine */
            color: #ddd;
        }
    )");

    // Заголовок
    QLabel *titleLabel = new QLabel("MindNote", this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    // Табы
    _tabWidget = new QTabWidget(this);
    _tabWidget->setTabPosition(QTabWidget::North);
    _tabWidget->setDocumentMode(true);
    _tabWidget->tabBar()->setStyleSheet("QTabBar::tab {"
                                        "   padding: 8px;"
                                        "   border: 1px solid #ddd;"
                                        "   background: #e0e0e0;"
                                        "   border-top-left-radius: 4px;"
                                        "   border-top-right-radius: 4px;"
                                        "}"
                                        "QTabBar::tab:selected {"
                                        "   background: #f5f5f5;"
                                        "   border-bottom-color: #f5f5f5;"
                                        "}");

    // Вкладка входа
    QWidget *loginTab = new QWidget(this);
    QVBoxLayout *loginLayout = new QVBoxLayout(loginTab);
    loginLayout->setContentsMargins(15, 15, 15, 15);
    loginLayout->setSpacing(15);

    // Поля входа
    _loginEmail = new QLineEdit(this);
    _loginEmail->setPlaceholderText("Email");

    _loginPassword = new QLineEdit(this);
    _loginPassword->setPlaceholderText("Пароль");
    _loginPassword->setEchoMode(QLineEdit::Password);

    _rememberMe = new QCheckBox("Запомнить меня", this);
    _rememberMe->setChecked(true);

    _loginButton = new QPushButton("Войти", this);
    _loginButton->setObjectName("loginButton");

    loginLayout->addWidget(new QLabel("Email:", this));
    loginLayout->addWidget(_loginEmail);
    loginLayout->addWidget(new QLabel("Пароль:", this));
    loginLayout->addWidget(_loginPassword);
    loginLayout->addWidget(_rememberMe);
    loginLayout->addWidget(_loginButton);
    loginLayout->addStretch();

    // Вкладка регистрации
    QWidget *registerTab = new QWidget(this);
    QVBoxLayout *registerLayout = new QVBoxLayout(registerTab);
    registerLayout->setContentsMargins(15, 15, 15, 15);
    registerLayout->setSpacing(15);

    // Поля регистрации
    _registerEmail = new QLineEdit(this);
    _registerEmail->setPlaceholderText("Email");

    _registerUsername = new QLineEdit(this);
    _registerUsername->setPlaceholderText("Имя пользователя");

    _registerPassword = new QLineEdit(this);
    _registerPassword->setPlaceholderText("Пароль");
    _registerPassword->setEchoMode(QLineEdit::Password);

    _registerButton = new QPushButton("Зарегистрироваться", this);
    _registerButton->setObjectName("registerButton");

    registerLayout->addWidget(new QLabel("Email:", this));
    registerLayout->addWidget(_registerEmail);
    registerLayout->addWidget(new QLabel("Имя пользователя:", this));
    registerLayout->addWidget(_registerUsername);
    registerLayout->addWidget(new QLabel("Пароль:", this));
    registerLayout->addWidget(_registerPassword);
    registerLayout->addWidget(_registerButton);

    // Добавляем вкладки
    _tabWidget->addTab(loginTab, "Вход");
    _tabWidget->addTab(registerTab, "Регистрация");
    mainLayout->addWidget(_tabWidget);

    // Разделитель
    QFrame *divider = new QFrame(this);
    divider->setFrameShape(QFrame::HLine);
    divider->setFrameShadow(QFrame::Plain);
    mainLayout->addWidget(divider);

    // Кнопка гостевого входа
    QPushButton *guestButton = new QPushButton("Продолжить как гость", this);
    guestButton->setObjectName("guestButton");
    mainLayout->addWidget(guestButton);

    // Соединения сигналов
    connect(_loginButton, &QPushButton::clicked, this, &AuthDialog::onLoginClicked);
    connect(_registerButton, &QPushButton::clicked, this, &AuthDialog::onRegisterClicked);
    connect(guestButton, &QPushButton::clicked, this, [this]() {
        emit guestLoginRequested();
        accept();
    });
}

void AuthDialog::closeEvent(QCloseEvent *event)
{
    // При закрытии окна просто игнорируем событие, чтобы не закрывать приложение
    event->ignore();
    reject(); // Отправляем сигнал об отмене авторизации
}

void AuthDialog::onLoginClicked()
{
    emit loginRequested(_loginEmail->text(), _loginPassword->text(), _rememberMe->isChecked());
}

void AuthDialog::onRegisterClicked()
{
    emit registerRequested(_registerEmail->text(), _registerPassword->text(),
                           _registerUsername->text());
}

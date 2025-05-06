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

    // Заголовок
    QLabel *titleLabel = new QLabel("MindNote", this);
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
    QWidget *loginTab = new QWidget();
    QVBoxLayout *loginLayout = new QVBoxLayout(loginTab);
    loginLayout->setContentsMargins(15, 15, 15, 15);
    loginLayout->setSpacing(15);

    // Поля входа
    _loginEmail = new QLineEdit();
    _loginEmail->setPlaceholderText("Email");
    _loginEmail->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 4px;");

    _loginPassword = new QLineEdit();
    _loginPassword->setPlaceholderText("Пароль");
    _loginPassword->setEchoMode(QLineEdit::Password);
    _loginPassword->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 4px;");

    _loginButton = new QPushButton("Войти");
    _loginButton->setStyleSheet("padding: 8px;"
                                "background-color: #4CAF50;"
                                "color: white;"
                                "border: none;"
                                "border-radius: 4px;");

    loginLayout->addWidget(new QLabel("Email:"));
    loginLayout->addWidget(_loginEmail);
    loginLayout->addWidget(new QLabel("Пароль:"));
    loginLayout->addWidget(_loginPassword);
    loginLayout->addWidget(_loginButton);
    loginLayout->addStretch();

    // Вкладка регистрации
    QWidget *registerTab = new QWidget();
    QVBoxLayout *registerLayout = new QVBoxLayout(registerTab);
    registerLayout->setContentsMargins(15, 15, 15, 15);
    registerLayout->setSpacing(15);

    // Поля регистрации
    _registerEmail = new QLineEdit();
    _registerEmail->setPlaceholderText("Email");
    _registerEmail->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 4px;");

    _registerUsername = new QLineEdit();
    _registerUsername->setPlaceholderText("Имя пользователя");
    _registerUsername->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 4px;");

    _registerPassword = new QLineEdit();
    _registerPassword->setPlaceholderText("Пароль");
    _registerPassword->setEchoMode(QLineEdit::Password);
    _registerPassword->setStyleSheet("padding: 8px; border: 1px solid #ccc; border-radius: 4px;");

    _registerButton = new QPushButton("Зарегистрироваться");
    _registerButton->setStyleSheet("padding: 8px;"
                                   "background-color: #2196F3;"
                                   "color: white;"
                                   "border: none;"
                                   "border-radius: 4px;");

    registerLayout->addWidget(new QLabel("Email:"));
    registerLayout->addWidget(_registerEmail);
    registerLayout->addWidget(new QLabel("Имя пользователя:"));
    registerLayout->addWidget(_registerUsername);
    registerLayout->addWidget(new QLabel("Пароль:"));
    registerLayout->addWidget(_registerPassword);
    registerLayout->addWidget(_registerButton);

    // Добавляем вкладки
    _tabWidget->addTab(loginTab, "Вход");
    _tabWidget->addTab(registerTab, "Регистрация");
    mainLayout->addWidget(_tabWidget);

    // Разделитель
    QFrame *divider = new QFrame();
    divider->setFrameShape(QFrame::HLine);
    divider->setFrameShadow(QFrame::Plain);
    divider->setStyleSheet("color: #ddd;");
    mainLayout->addWidget(divider);

    // Кнопка гостевого входа
    QPushButton *guestButton = new QPushButton("Продолжить как гость");
    guestButton->setStyleSheet("padding: 8px;"
                               "background-color: #9E9E9E;"
                               "color: white;"
                               "border: none;"
                               "border-radius: 4px;");
    mainLayout->addWidget(guestButton);

    // Соединения сигналов
    connect(_loginButton, &QPushButton::clicked, this, &AuthDialog::onLoginClicked);
    connect(_registerButton, &QPushButton::clicked, this, &AuthDialog::onRegisterClicked);
    connect(guestButton, &QPushButton::clicked, this, [this]() {
        emit guestLoginRequested();
        accept();
    });

    // Настройка стилей без теней
    setStyleSheet(R"(
        QDialog {
            background-color: #f5f5f5;
        }
        QTabWidget::pane {
            border: 1px solid #ddd;
            border-radius: 4px;
            margin-top: -1px;
        }
    )");
}

void AuthDialog::closeEvent(QCloseEvent *event)
{
    // При закрытии окна просто игнорируем событие, чтобы не закрывать приложение
    event->ignore();
    reject(); // Отправляем сигнал об отмене авторизации
}

void AuthDialog::onLoginClicked()
{
    emit loginRequested(_loginEmail->text(), _loginPassword->text());
}

void AuthDialog::onRegisterClicked()
{
    emit registerRequested(_loginEmail->text(), _loginPassword->text(), _registerUsername->text());
}

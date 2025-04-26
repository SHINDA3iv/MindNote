#include "main_widget.h"
#include "auth_dialog.h"
#include <qtoolbar.h>

#include <QLayout>
#include <QCoreApplication>
#include <QMessageBox>

MainWidget::MainWidget(QWidget *parent) : QWidget(parent)
{
    QString settingsPath = QCoreApplication::applicationDirPath() + "/settings.ini";
    _settings = new QSettings(settingsPath, QSettings::IniFormat, this);

    _localStorage = std::make_shared<LocalStorage>();
    _apiClient = std::make_shared<ApiClient>();
    _syncManager = std::make_shared<SyncManager>(_apiClient, _localStorage);

    _workspaceController = std::make_unique<WorkspaceController>(_localStorage);

    _authManager = std::make_shared<AuthManager>();

    if (!_authManager->isAuthenticated()) {
        showAuthDialog();
    } else {
        initApplication();
    }
}

void MainWidget::showAuthDialog()
{
    AuthDialog *authDialog = new AuthDialog(this);
    authDialog->setWindowModality(Qt::ApplicationModal);

    connect(authDialog, &AuthDialog::loginRequested, _apiClient.get(), &ApiClient::login);
    connect(authDialog, &AuthDialog::registerRequested, _apiClient.get(), &ApiClient::registerUser);
    connect(authDialog, &AuthDialog::guestLoginRequested, this, [this]() {
        _isGuestMode = true;
        initApplication();
    });

    connect(_apiClient.get(), &ApiClient::loginSuccess, this,
            [this, authDialog](const QString &token) {
                _isGuestMode = false;
                _authManager->login(token, "");
                authDialog->accept();
                initApplication();
            });

    connect(_apiClient.get(), &ApiClient::loginFailed, this, [authDialog](const QString &error) {
        QMessageBox::warning(authDialog, "Ошибка входа", error);
    });

    connect(_apiClient.get(), &ApiClient::registrationSuccess, this, [authDialog]() {
        QMessageBox::information(authDialog, "Успешная регистрация",
                                 "Регистрация завершена. Теперь вы можете войти.");
        authDialog->setCurrentTab(0);
    });

    connect(_apiClient.get(), &ApiClient::registrationFailed, this,
            [authDialog](const QString &error) {
                QMessageBox::warning(authDialog, "Ошибка регистрации", error);
            });

    // Обработка закрытия диалога
    connect(authDialog, &QDialog::rejected, this, [this]() {
        // Если пользователь закрыл диалог, продолжаем как гость
        if (!_authManager->isAuthenticated() && !_isGuestMode) {
            _isGuestMode = true;
            initApplication();
        }
    });

    authDialog->exec();
    authDialog->deleteLater();
}

void MainWidget::initApplication()
{
    if (!_isGuestMode) {
        _apiClient->setAuthToken(_authManager->getAuthToken());
        _syncManager->startAutoSync(5 * 60 * 1000); // Синхронизация только для авторизованных
    }

    // Инициализация остального приложения
    initWindow();
    initConnections();
    restoreSettings();
}

MainWidget::~MainWidget()
{
    _workspaceController->saveWorkspaces();
    saveSettings();
}

void MainWidget::onLoginRequested()
{
    AuthDialog *authDialog = new AuthDialog(this);
    authDialog->setCurrentTab(0); // Вкладка входа

    connect(authDialog, &AuthDialog::loginRequested, _apiClient.get(), &ApiClient::login);

    connect(_apiClient.get(), &ApiClient::loginSuccess, this,
            [this, authDialog](const QString &token) {
                _isGuestMode = false;
                _authManager->login(token, "");
                authDialog->accept();

                // Обновляем интерфейс после входа
                _loginButton->setVisible(false);
                _logoutButton->setVisible(true);
                _syncAction->setVisible(true);

                // Загружаем данные с сервера
                _syncManager->performFullSync();
            });

    connect(_apiClient.get(), &ApiClient::loginFailed, this, [authDialog](const QString &error) {
        QMessageBox::warning(authDialog, "Ошибка входа", error);
    });

    authDialog->exec();
    authDialog->deleteLater();
}

void MainWidget::onLogout()
{
    if (QMessageBox::question(this, "Подтверждение выхода",
                              "Вы уверены, что хотите выйти из аккаунта?",
                              QMessageBox::Yes | QMessageBox::No)
        == QMessageBox::Yes) {
        _authManager->logout();
        _apiClient->setAuthToken("");
        _isGuestMode = true;

        // Обновляем интерфейс
        _loginButton->setVisible(true);
        _logoutButton->setVisible(false);
        _syncAction->setVisible(false);

        // Очищаем данные (или оставляем локальные изменения)
        _workspaceController.reset(new WorkspaceController(_localStorage));
        _leftPanel->setWorkspaceController(_workspaceController.get());
        _leftPanel->refreshWorkspaceList();
        _editorWidget->setCurrentWorkspace(nullptr);
    }
}

void MainWidget::initWindow()
{
    this->setStyleSheet(R"(
        /* Общие стили */
        QWidget {
            font-family: 'Segoe UI', Arial, sans-serif;
            font-size: 14px;
        }

        /* Кнопки */
        QPushButton, QToolButton {
            padding: 8px 12px;
            border: none;
            border-radius: 4px;
            background-color: #f0f0f0;
            min-width: 80px;
        }

        QPushButton:hover, QToolButton:hover {
            background-color: #e0e0e0;
        }

        /* Списки */
        QListWidget {
            border: 1px solid #ddd;
            border-radius: 4px;
            background-color: white;
            alternate-background-color: #f9f9f9;
        }

        QListWidget::item {
            padding: 8px;
            border-bottom: 1px solid #eee;
        }

        QListWidget::item:selected {
            background-color: #e0f0ff;
            color: black;
        }

        /* Разделители */
        QSplitter::handle {
            background-color: #e0e0e0;
            width: 4px;
        }

        /* Вкладки */
        QTabBar::tab {
            padding: 8px 12px;
            border: 1px solid #ddd;
            background: #f0f0f0;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }

        QTabBar::tab:selected {
            background: white;
            border-bottom-color: white;
        }
    )");

    _leftPanel = std::make_unique<LeftPanel>(this);
    _editorWidget = std::make_unique<EditorWidget>(this);

    // Настройка панели инструментов
    QToolBar *toolBar = new QToolBar(this);
    toolBar->setMovable(false);

    _syncAction = toolBar->addAction("Синхронизировать");
    _syncAction->setIcon(QIcon(":/icons/sync.png"));
    _syncAction->setVisible(!_isGuestMode);
    connect(_syncAction, &QAction::triggered, this, [this]() {
        _syncManager->performFullSync();
    });

    _loginButton = new QToolButton(this);
    _loginButton->setText("Войти");
    _loginButton->setIcon(QIcon(":/icons/login.png"));
    _loginButton->setVisible(_isGuestMode);
    connect(_loginButton, &QToolButton::clicked, this, &MainWidget::onLoginRequested);
    toolBar->addWidget(_loginButton);

    _logoutButton = new QToolButton(this);
    _logoutButton->setText("Выйти");
    _logoutButton->setVisible(!_isGuestMode);
    _logoutButton->setIcon(QIcon::fromTheme("system-log-out"));
    connect(_logoutButton, &QToolButton::clicked, this, &MainWidget::onLogout);
    toolBar->addWidget(_logoutButton);

    QString buttonStyle = R"(
    QToolButton {
        padding: 6px 12px;
        border: 1px solid #ddd;
        border-radius: 4px;
        background-color: %1;
        color: %2;
    }
    QToolButton:hover {
        background-color: %3;
    }
)";

    _loginButton->setStyleSheet(buttonStyle.arg("#4CAF50").arg("white").arg("#45a049"));
    _logoutButton->setStyleSheet(buttonStyle.arg("#f44336").arg("white").arg("#d32f2f"));
    _syncAction->setIcon(QIcon(":/icons/sync.png"));

    // Настройка главного разделителя
    _mainSplitter = new QSplitter(Qt::Horizontal, this);
    _mainSplitter->addWidget(_leftPanel.get());
    _mainSplitter->addWidget(_editorWidget.get());
    _mainSplitter->setStretchFactor(1, 1); // Редактор занимает больше места

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);
    mainLayout->addWidget(toolBar);
    mainLayout->addWidget(_mainSplitter);
    setLayout(mainLayout);

    _leftPanel->setWorkspaceController(_workspaceController.get());
}

void MainWidget::initConnections()
{
    connect(_leftPanel.get(), &LeftPanel::workspaceSelected, _editorWidget.get(),
            &EditorWidget::setCurrentWorkspace);
}

void MainWidget::saveSettings()
{
    _settings->beginGroup("MainWidget");
    _settings->setValue("splitterState", _mainSplitter->saveState());
    _settings->endGroup();
}

void MainWidget::restoreSettings()
{
    _settings->beginGroup("MainWidget");
    QByteArray splitterState = _settings->value("splitterState").toByteArray();
    if (!splitterState.isEmpty()) {
        _mainSplitter->restoreState(splitterState);
    }
    _settings->endGroup();
}

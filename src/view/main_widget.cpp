#include "main_widget.h"
#include "auth_dialog.h"

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

    QPushButton *guestButton = new QPushButton("Продолжить как гость", authDialog);
    authDialog->layout()->addWidget(guestButton);

    connect(guestButton, &QPushButton::clicked, this, [this, authDialog]() {
        _isGuestMode = true;
        authDialog->accept();
        initApplication();
    });

    connect(authDialog, &AuthDialog::loginRequested, _apiClient.get(), &ApiClient::login);
    connect(authDialog, &AuthDialog::registerRequested, _apiClient.get(), &ApiClient::registerUser);

    connect(
     _apiClient.get(), &ApiClient::loginSuccess, this, [this, authDialog](const QString &token) {
         _isGuestMode = false;
         _authManager->login(token, ""); // В реальном приложении нужно передать email и пароль
         authDialog->accept();
         initApplication();
     });

    connect(_apiClient.get(), &ApiClient::loginFailed, this, [authDialog](const QString &error) {
        QMessageBox::warning(authDialog, "Login Failed", error);
    });

    connect(_apiClient.get(), &ApiClient::registrationSuccess, this, [authDialog]() {
        QMessageBox::information(authDialog, "Registration",
                                 "Registration successful. Please login.");
        authDialog->setCurrentTab(0);
    });

    connect(_apiClient.get(), &ApiClient::registrationFailed, this,
            [authDialog](const QString &error) {
                QMessageBox::warning(authDialog, "Registration Failed", error);
            });

    authDialog->exec();
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

void MainWidget::onLogout()
{
    if (_isGuestMode)
        return;

    _authManager->logout();
    _apiClient->setAuthToken("");
    _isGuestMode = true;

    // Очищаем интерфейс
    _leftPanel.reset();
    _editorWidget.reset();

    // Показываем диалог авторизации
    showAuthDialog();
}

void MainWidget::initWindow()
{
    _leftPanel = std::make_unique<LeftPanel>(this);
    _editorWidget = std::make_unique<EditorWidget>(this);

    _leftPanel->setWorkspaceController(_workspaceController.get());

    _mainSplitter = new QSplitter(this);
    _mainSplitter->addWidget(_leftPanel.get());
    _mainSplitter->addWidget(_editorWidget.get());

    QToolButton *logoutButton = new QToolButton(this);
    logoutButton->setText("Logout");
    logoutButton->setVisible(!_isGuestMode);
    connect(logoutButton, &QToolButton::clicked, this, &MainWidget::onLogout);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(logoutButton);
    mainLayout->addWidget(_mainSplitter);
    setLayout(mainLayout);
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

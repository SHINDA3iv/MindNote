#include "main_widget.h"
#include "auth_dialog.h"
#include <qtoolbar.h>

#include <QLayout>
#include <QCoreApplication>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QApplication>

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , _hasUnsavedChanges(false)
    , _zoomFactor(1.0)
    , _sidebarVisible(true)
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

    initUI();
    createToolbars();
    createStatusBar();
    initConnections();
}

MainWidget::~MainWidget()
{
    if (_workspaceController) {
        _workspaceController->saveWorkspaces();
    }
    saveSettings();
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

void MainWidget::initUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Create toolbars
    _mainToolBar = new QToolBar(this);
    _editToolBar = new QToolBar(this);
    _viewToolBar = new QToolBar(this);

    mainLayout->addWidget(_mainToolBar);
    mainLayout->addWidget(_editToolBar);
    mainLayout->addWidget(_viewToolBar);

    // Create main splitter
    _mainSplitter = new QSplitter(Qt::Horizontal, this);
    _sidebar = new QWidget(_mainSplitter);
    _workspaceArea = new QScrollArea(_mainSplitter);

    _mainSplitter->addWidget(_sidebar);
    _mainSplitter->addWidget(_workspaceArea);
    _mainSplitter->setStretchFactor(0, 1);
    _mainSplitter->setStretchFactor(1, 3);

    mainLayout->addWidget(_mainSplitter);

    // Create status bar
    _statusBar = new QStatusBar(this);
    mainLayout->addWidget(_statusBar);
}

void MainWidget::createToolbars()
{
    // Main toolbar actions
    _mainToolBar->addAction(QIcon(":/icons/add.png"), "New", this, &MainWidget::createNewWorkspace);
    _mainToolBar->addAction(QIcon(":/icons/save.png"), "Save", this, &MainWidget::saveCurrentWorkspace);
    _mainToolBar->addAction(QIcon(":/icons/save-as.png"), "Save As", this, &MainWidget::saveWorkspaceAs);
    _mainToolBar->addSeparator();
    _mainToolBar->addAction(QIcon(":/icons/sync.png"), "Sync", this, &MainWidget::syncWorkspaces);

    // Edit toolbar actions
    _editToolBar->addAction(QIcon(":/icons/undo.png"), "Undo", this, &MainWidget::undo);
    _editToolBar->addAction(QIcon(":/icons/redo.png"), "Redo", this, &MainWidget::redo);
    _editToolBar->addSeparator();
    _editToolBar->addAction(QIcon(":/icons/cut.png"), "Cut", this, &MainWidget::cut);
    _editToolBar->addAction(QIcon(":/icons/copy.png"), "Copy", this, &MainWidget::copy);
    _editToolBar->addAction(QIcon(":/icons/paste.png"), "Paste", this, &MainWidget::paste);

    // View toolbar actions
    _viewToolBar->addAction(QIcon(":/icons/sidebar.png"), "Toggle Sidebar", this, &MainWidget::toggleSidebar);
    _viewToolBar->addSeparator();
    _viewToolBar->addAction("Zoom In", this, &MainWidget::zoomIn);
    _viewToolBar->addAction("Zoom Out", this, &MainWidget::zoomOut);
    _viewToolBar->addAction("Reset Zoom", this, &MainWidget::zoomReset);
}

void MainWidget::createStatusBar()
{
    _statusBar->showMessage("Ready");
}

void MainWidget::initConnections()
{
    connect(_leftPanel.get(), &LeftPanel::workspaceSelected, _editorWidget.get(),
            &EditorWidget::setCurrentWorkspace);
}

// Workspace operations
void MainWidget::createNewWorkspace()
{
    if (hasUnsavedChanges()) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Unsaved Changes",
            "Do you want to save your changes before creating a new workspace?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (reply == QMessageBox::Save) {
            if (!saveCurrentWorkspace()) {
                return;
            }
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }

    // TODO: Implement new workspace creation
    _hasUnsavedChanges = false;
    _statusBar->showMessage("New workspace created");
}

void MainWidget::openWorkspace()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Workspace",
        QString(), "Workspace Files (*.workspace);;All Files (*)");

    if (!fileName.isEmpty()) {
        // TODO: Implement workspace loading
        _hasUnsavedChanges = false;
        _statusBar->showMessage("Workspace opened: " + fileName);
    }
}

bool MainWidget::saveCurrentWorkspace()
{
    // TODO: Implement workspace saving
    _hasUnsavedChanges = false;
    _statusBar->showMessage("Workspace saved");
    return true;
}

bool MainWidget::saveWorkspaceAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save Workspace As",
        QString(), "Workspace Files (*.workspace);;All Files (*)");

    if (!fileName.isEmpty()) {
        // TODO: Implement workspace saving
        _hasUnsavedChanges = false;
        _statusBar->showMessage("Workspace saved as: " + fileName);
        return true;
    }
    return false;
}

void MainWidget::syncWorkspaces()
{
    // TODO: Implement workspace synchronization
    _statusBar->showMessage("Workspaces synchronized");
}

// Edit operations
void MainWidget::undo()
{
    // TODO: Implement undo
    _statusBar->showMessage("Undo");
}

void MainWidget::redo()
{
    // TODO: Implement redo
    _statusBar->showMessage("Redo");
}

void MainWidget::cut()
{
    // TODO: Implement cut
    _statusBar->showMessage("Cut");
}

void MainWidget::copy()
{
    // TODO: Implement copy
    _statusBar->showMessage("Copy");
}

void MainWidget::paste()
{
    // TODO: Implement paste
    _statusBar->showMessage("Paste");
}

// View operations
void MainWidget::zoomIn()
{
    _zoomFactor *= 1.1;
    // TODO: Implement zoom
    _statusBar->showMessage(QString("Zoom: %1%").arg(int(_zoomFactor * 100)));
}

void MainWidget::zoomOut()
{
    _zoomFactor /= 1.1;
    // TODO: Implement zoom
    _statusBar->showMessage(QString("Zoom: %1%").arg(int(_zoomFactor * 100)));
}

void MainWidget::zoomReset()
{
    _zoomFactor = 1.0;
    // TODO: Implement zoom reset
    _statusBar->showMessage("Zoom reset to 100%");
}

void MainWidget::toggleSidebar()
{
    _sidebarVisible = !_sidebarVisible;
    _sidebar->setVisible(_sidebarVisible);
    _statusBar->showMessage(_sidebarVisible ? "Sidebar shown" : "Sidebar hidden");
}

bool MainWidget::hasUnsavedChanges() const
{
    return _hasUnsavedChanges;
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

void MainWidget::restoreSettings()
{
    _settings->beginGroup("MainWidget");
    QByteArray splitterState = _settings->value("splitterState").toByteArray();
    if (!splitterState.isEmpty()) {
        _mainSplitter->restoreState(splitterState);
    }
    _settings->endGroup();
}

void MainWidget::saveSettings()
{
    if (_settings) {
        _settings->beginGroup("MainWidget");
        if (_mainSplitter) {
            _settings->setValue("splitterState", _mainSplitter->saveState());
        }
        _settings->setValue("sidebarVisible", _sidebarVisible);
        _settings->setValue("zoomFactor", _zoomFactor);
        _settings->endGroup();
    }
}

#include "main_widget.h"
#include "auth_dialog.h"
#include "../error_handler.h"
#include <qtoolbar.h>

#include <QLayout>
#include <QCoreApplication>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QApplication>
#include <QDebug>
#include <QTimer>
#include <QProgressDialog>

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    _hasUnsavedChanges(false),
    _zoomFactor(1.0),
    _sidebarVisible(true)
{
    QString settingsPath = QCoreApplication::applicationDirPath() + "/settings.ini";
    _settings = new QSettings(settingsPath, QSettings::IniFormat, this);

    _localStorage = std::make_shared<LocalStorage>(this);
    _apiClient = std::make_shared<ApiClient>(this);
    _syncManager = std::make_shared<SyncManager>(_apiClient, _localStorage, this);
    _workspaceController = std::make_unique<WorkspaceController>(_localStorage, this);
    _authManager = std::make_shared<AuthManager>(this);

    // Connect workspace controller signals
    connect(_workspaceController.get(), &WorkspaceController::workspaceAdded, this,
            &MainWidget::workspaceAdded);
    connect(_workspaceController.get(), &WorkspaceController::workspaceRemoved, this,
            &MainWidget::workspaceRemoved);
    connect(_workspaceController.get(), &WorkspaceController::pathUpdated, this,
            [this](Workspace *workspace) {
                if (_editorWidget) {
                    _editorWidget->setCurrentWorkspace(workspace);
                }
            });

    // Check authentication after UI is initialized
    if (!_authManager->isAuthenticated()) {
        showAuthDialog();
    } else {
        initApplication();
    }

    // Update UI state
    updateAuthUI();
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
    qDebug() << "Showing auth dialog";
    auto authDialog = new AuthDialog(this);
    authDialog->setWindowTitle("Авторизация");
    authDialog->setWindowModality(Qt::ApplicationModal);

    connect(authDialog, &AuthDialog::loginRequested, this,
            [this, authDialog](const QString &username, const QString &password, bool rememberMe) {
                qDebug() << "Login requested - username:" << username
                         << "rememberMe:" << rememberMe;
                _apiClient->login(username, password);
                _authManager->setRememberMe(rememberMe);
            });
    connect(authDialog, &AuthDialog::registerRequested, _apiClient.get(), &ApiClient::registerUser);
    connect(authDialog, &AuthDialog::guestLoginRequested, this, [this]() {
        _isGuestMode = true;
        initApplication();
    });

    connect(
     _apiClient.get(), &ApiClient::loginSuccess, this, [this, authDialog](const QString &token) {
         _authManager->login(token, _apiClient->getUsername(), _authManager->isRememberMeEnabled());

         // Устанавливаем пользователя для LocalStorage
         _localStorage->setCurrentUser(_authManager->getUsername());

         // --- СИНХРОНИЗАЦИЯ ГОСТЕВЫХ ПРОСТРАНСТВ ---
         QProgressDialog progress("Синхронизация рабочих пространств...", QString(), 0, 0, this);
         progress.setWindowModality(Qt::ApplicationModal);
         progress.setCancelButton(nullptr);
         progress.setMinimumDuration(0);
         progress.show();

         connect(_syncManager.get(), &SyncManager::syncCompleted, &progress,
                 &QProgressDialog::close);
         connect(_syncManager.get(), &SyncManager::syncError, &progress, &QProgressDialog::close);

         _syncManager->startUserSync();

         // После syncCompleted/initApplication
         connect(_syncManager.get(), &SyncManager::syncCompleted, this, [this, authDialog]() {
             initApplication();
             authDialog->accept();
         });
         connect(_syncManager.get(), &SyncManager::syncError, this,
                 [authDialog](const QString &err) {
                     ErrorHandler::instance().showError("Ошибка синхронизации", err);
                     authDialog->accept();
                 });
     });
    connect(_apiClient.get(), &ApiClient::loginError, this, [authDialog](const QString &error) {
        authDialog->showLoginError(error);
        ErrorHandler::instance().showError("Ошибка входа", error);
    });
    connect(_apiClient.get(), &ApiClient::registerSuccess, this, [authDialog]() {
        ErrorHandler::instance().showInfo("Успех", "Регистрация успешна");
        authDialog->accept();
    });
    connect(_apiClient.get(), &ApiClient::registerError, this, [authDialog](const QString &error) {
        authDialog->showRegisterError(error);
        ErrorHandler::instance().showError("Ошибка регистрации", error);
    });

    // Handle workspace synchronization after login
    connect(_apiClient.get(), &ApiClient::workspacesReceived, this,
            [this](const QJsonArray &serverWorkspaces) {
                if (_authManager->isRememberMeEnabled()) {
                    // Show dialog to choose synchronization strategy
                    QMessageBox::StandardButton reply = QMessageBox::question(
                     this, "Синхронизация пространств",
                     "Обнаружены различия между локальными и серверными пространствами. "
                     "Хотите сохранить локальные версии?",
                     QMessageBox::Yes | QMessageBox::No);

                    if (reply == QMessageBox::Yes) {
                        // Keep local versions
                        _localStorage->syncWorkspaces(serverWorkspaces, true);
                    } else {
                        // Use server versions
                        _localStorage->syncWorkspaces(serverWorkspaces, false);
                    }
                } else {
                    // If remember me is not enabled, only sync to server
                    _localStorage->syncWorkspaces(serverWorkspaces, false);
                }
            });

    // Обработка закрытия диалога
    connect(authDialog, &QDialog::rejected, this, [this]() {
        // Если пользователь закрыл диалог, продолжаем как гость
        if (!_authManager->isAuthenticated() && !_isGuestMode) {
            QApplication::quit();
        }
    });

    qDebug() << "Executing auth dialog";
    authDialog->exec();
    qDebug() << "Auth dialog finished";
    authDialog->deleteLater();
}

void MainWidget::initApplication()
{
    // Устанавливаем пользователя для LocalStorage, если не гость
    if (!_isGuestMode) {
        _localStorage->setCurrentUser(_authManager->getUsername());
    }
    // Инициализация остального приложения
    initWindow();
    initConnections();
    restoreSettings();
    _workspaceController->loadWorkspaces();
    updateWorkspaceList();

    if (!_isGuestMode) {
        _apiClient->setAuthToken(_authManager->getAuthToken());

        // Проверяем токен на сервере при запуске
        _apiClient->getCurrentUser();

        // Обработка результата проверки токена
        connect(_apiClient.get(), &ApiClient::error, this, [this](const QString &err) {
            QMessageBox::warning(this, "Сессия истекла",
                                 "Ваша сессия истекла. Пожалуйста, войдите снова.");
            _authManager->logout();
            showAuthDialog();
        });

        _leftPanel->refreshWorkspaceList();
    }
}

void MainWidget::checkToken()
{
    if (_authManager->isAuthenticated()) {
        _apiClient->getCurrentUser();
    }
}

void MainWidget::onLoginRequested()
{
    showAuthDialog();
}

void MainWidget::onLogout()
{
    _authManager->logout();
    updateAuthUI();
}

void MainWidget::initConnections()
{
    connect(_workspaceController.get(), &WorkspaceController::workspaceRemoved, _editorWidget.get(),
            &EditorWidget::onWorkspaceRemoved);

    connect(_leftPanel.get(), &LeftPanel::workspaceSelected, _editorWidget.get(),
            &EditorWidget::setCurrentWorkspace);
    connect(_leftPanel.get(), &LeftPanel::subWorkspaceSelected, _editorWidget.get(),
            &EditorWidget::setCurrentWorkspace);

    // Auth connections
    connect(_authManager.get(), &AuthManager::authStateChanged, this,
            &MainWidget::onAuthStateChanged);
    connect(_authManager.get(), &AuthManager::loginRequested, this, &MainWidget::onLoginRequested);
    connect(_authManager.get(), &AuthManager::logoutRequested, this, &MainWidget::onLogout);

    // Token validation connections
    connect(_apiClient.get(), &ApiClient::error, this, [this](const QString &err) {
        QMessageBox::warning(this, "Сессия истекла",
                             "Ваша сессия истекла. Пожалуйста, войдите снова.");
        _authManager->logout();
        showAuthDialog();
    });

    // Sync connections
    connect(_syncManager.get(), &SyncManager::versionConflictDetected, this,
            &MainWidget::onVersionConflictDetected);
    connect(_syncManager.get(), &SyncManager::syncCompleted, this, &MainWidget::onSyncCompleted);
    connect(_syncManager.get(), &SyncManager::syncError, this, &MainWidget::onSyncError);
}

void MainWidget::onAuthStateChanged()
{
    updateAuthUI();
    emit authStateChanged();
}

void MainWidget::updateAuthUI()
{
    bool isAuthenticated = _authManager->isAuthenticated();
    QString username = _authManager->getUsername();

    qDebug() << "Updating auth UI - isAuthenticated:" << isAuthenticated << "username:" << username;

    // Update status message
    if (isAuthenticated) {
        emit statusMessage(tr("Logged in as: %1").arg(username));
    } else {
        emit statusMessage(tr("Not logged in"));
    }

    // Emit auth state changed to update main window UI
    emit authStateChanged();
}

void MainWidget::onVersionConflictDetected(const QJsonArray &serverWorkspaces)
{
    QMessageBox::StandardButton reply =
     QMessageBox::question(this, "Конфликт версий",
                           "Обнаружены различия между локальными и серверными пространствами. "
                           "Хотите сохранить локальные версии?",
                           QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // Keep local versions
        _localStorage->syncWorkspaces(serverWorkspaces, true);
    } else {
        // Use server versions
        _localStorage->syncWorkspaces(serverWorkspaces, false);
    }
}

void MainWidget::onSyncCompleted()
{
    emit statusMessage(tr("Синхронизация завершена"));
    updateWorkspaceList();
}

void MainWidget::onSyncError(const QString &error)
{
    emit statusMessage(tr("Ошибка синхронизации: %1").arg(error));
    ErrorHandler::instance().showError("Ошибка синхронизации", error);
}

void MainWidget::syncWorkspaces()
{
    if (!_authManager->isAuthenticated()) {
        emit statusMessage(tr("Требуется авторизация для синхронизации"));
        return;
    }

    _syncManager->performFullSync();
}

// Workspace operations
void MainWidget::createNewWorkspace()
{
    if (hasUnsavedChanges()) {
        QMessageBox::StandardButton reply =
         QMessageBox::question(this, "Unsaved Changes",
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
    emit statusMessage("New workspace created");
}

void MainWidget::openWorkspace()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Workspace", QString(),
                                                    "Workspace Files (*.workspace);;All Files (*)");

    if (!fileName.isEmpty()) {
        // TODO: Implement workspace loading
        _hasUnsavedChanges = false;
        emit statusMessage("Workspace opened: " + fileName);
    }
}

bool MainWidget::saveCurrentWorkspace()
{
    if (!_editorWidget) {
        qWarning() << "No editor widget available for saving workspace";
        return false;
    }

    Workspace *currentWorkspace = _editorWidget->currentWorkspace();
    if (!currentWorkspace) {
        qWarning() << "No current workspace to save";
        return false;
    }

    qDebug() << "Saving current workspace:" << currentWorkspace->getTitle();

    if (_workspaceController) {
        _workspaceController->saveWorkspaces();
        _hasUnsavedChanges = false;
        emit statusMessage("Workspace saved: " + currentWorkspace->getTitle());
        return true;
    }

    qWarning() << "Workspace controller not available for saving";
    return false;
}

bool MainWidget::saveWorkspaceAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save Workspace As", QString(),
                                                    "Workspace Files (*.workspace);;All Files (*)");

    if (!fileName.isEmpty()) {
        // TODO: Implement workspace saving
        _hasUnsavedChanges = false;
        emit statusMessage("Workspace saved as: " + fileName);
        return true;
    }
    return false;
}

// Edit operations
void MainWidget::undo()
{
    // TODO: Implement undo
    emit statusMessage("Undo");
}

void MainWidget::redo()
{
    // TODO: Implement redo
    emit statusMessage("Redo");
}

void MainWidget::cut()
{
    // TODO: Implement cut
    emit statusMessage("Cut");
}

void MainWidget::copy()
{
    // TODO: Implement copy
    emit statusMessage("Copy");
}

void MainWidget::paste()
{
    // TODO: Implement paste
    emit statusMessage("Paste");
}

// View operations
void MainWidget::zoomIn()
{
    _zoomFactor *= 1.1;
    // TODO: Implement zoom
    emit statusMessage(QString("Zoom: %1%").arg(int(_zoomFactor * 100)));
}

void MainWidget::zoomOut()
{
    _zoomFactor /= 1.1;
    // TODO: Implement zoom
    emit statusMessage(QString("Zoom: %1%").arg(int(_zoomFactor * 100)));
}

void MainWidget::zoomReset()
{
    _zoomFactor = 1.0;
    // TODO: Implement zoom reset
    emit statusMessage("Zoom reset to 100%");
}

void MainWidget::toggleSidebar()
{
    _sidebarVisible = !_sidebarVisible;
    _leftPanel->setVisible(_sidebarVisible);

    // Set the left panel size to zero when hiding the sidebar
    if (!_sidebarVisible) {
        // Save current width before hiding
        _lastPanelWidth = _mainSplitter->sizes().first();
        _mainSplitter->setSizes(QList<int> { 0, _mainSplitter->width() });
    } else {
        // Restore last known width when showing
        _mainSplitter->setSizes(
         QList<int> { _lastPanelWidth, _mainSplitter->width() - _lastPanelWidth });
    }

    emit statusMessage(_sidebarVisible ? "Sidebar shown" : "Sidebar hidden");
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

        /* Разделители */
        QSplitter {
            border: 2px solid #e0e0e0;
            border-radius: 10px;
        }

        QSplitter::handle {
            background-color: #e0e0e0;
            width: 8px;
        }
    )");

    _leftPanel = std::make_unique<LeftPanel>(this);
    _editorWidget = std::make_unique<EditorWidget>(this);

    // Настройка главного разделителя
    _mainSplitter = new QSplitter(Qt::Horizontal, this);
    _mainSplitter->setContentsMargins(0, 0, 0, 0);
    _mainSplitter->setHandleWidth(1);
    _mainSplitter->addWidget(_leftPanel.get());
    _mainSplitter->addWidget(_editorWidget.get());
    _mainSplitter->setStretchFactor(1, 1); // Редактор занимает больше места

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
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

bool MainWidget::isAuthenticated() const
{
    return _authManager->isAuthenticated();
}

QString MainWidget::getUsername() const
{
    return _authManager->getUsername();
}

void MainWidget::logout()
{
    _authManager->logout();
    updateAuthUI();
}

void MainWidget::updateWorkspaceList()
{
    if (auto leftPanel = findChild<LeftPanel *>()) {
        leftPanel->updateWorkspaceList();
    }
}

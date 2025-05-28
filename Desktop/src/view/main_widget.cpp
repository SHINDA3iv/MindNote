#include "main_widget.h"
#include "../error_handler.h"
#include "auth_dialog.h"
#include <qmainwindow.h>
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
        qDebug() << "SHITmain";
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
    this->hide();
    qDebug() << "Showing auth dialog";
    _pendingAuthDialog = new AuthDialog(this);
    _pendingAuthDialog->setWindowTitle("Авторизация");
    _pendingAuthDialog->setWindowModality(Qt::ApplicationModal);
    _pendingAuthType.clear();

    connect(_pendingAuthDialog, &AuthDialog::loginRequested, this,
            [this](const QString &username, const QString &password, bool rememberMe) {
                qDebug() << "Login requested - username:" << username << "rememberMe:" << rememberMe;
                _pendingAuthType = "login";
                _apiClient->login(username, password);
                _authManager->setRememberMe(rememberMe);
            });
    connect(_pendingAuthDialog, &AuthDialog::registerRequested, _apiClient.get(), &ApiClient::registerUser);
    connect(_pendingAuthDialog, &AuthDialog::guestLoginRequested, this, [this]() {
        _pendingAuthType = "guest";
        _isGuestMode = true;
        initApplication();
        this->show();
        if (_pendingAuthDialog) _pendingAuthDialog->accept();
    });
    // Обработка закрытия диалога
    connect(_pendingAuthDialog, &QDialog::rejected, this, [this]() {
        qDebug() << "Auth dialog rejected, isAuthenticated:" << _authManager->isAuthenticated()
                 << "isGuest:" << _isGuestMode;
        if (!_authManager->isAuthenticated() && !_isGuestMode) {
            qDebug() << "QApplication::quit() called from auth dialog rejected";
            QApplication::quit();
        }
    });
    _pendingAuthDialog->exec();
    qDebug() << "Auth dialog finished";
    if (_pendingAuthDialog) _pendingAuthDialog->deleteLater();
    if (_authManager->isAuthenticated() && !_isGuestMode) {
        this->show();
    }
    _pendingAuthDialog = nullptr;
    _pendingAuthType.clear();
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
            qDebug() << "SHITinitApplication";
            showAuthDialog();
        });
    }

    _leftPanel->refreshWorkspaceList();
}

void MainWidget::checkToken()
{
    if (_authManager->isAuthenticated()) {
        _apiClient->getCurrentUser();
    }
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

    // Token validation connections
    connect(_apiClient.get(), &ApiClient::error, this, [this](const QString &err) {
        QMessageBox::warning(this, "Сессия истекла",
                             "Ваша сессия истекла. Пожалуйста, войдите снова.");
        _authManager->logout();
        qDebug() << "SHITinitConnections";
        showAuthDialog();
    });

    // Sync connections
    connect(_syncManager.get(), &SyncManager::versionConflictDetected, this,
            &MainWidget::onVersionConflictDetected);
    connect(_syncManager.get(), &SyncManager::syncCompleted, this, &MainWidget::onSyncCompletedAuth);
    connect(_syncManager.get(), &SyncManager::syncError, this, &MainWidget::onSyncErrorAuth);

    connect(_apiClient.get(), &ApiClient::loginSuccess, this, &MainWidget::onLoginSuccess);
    connect(_apiClient.get(), &ApiClient::loginError, this, [this](const QString &error) {
        if (_pendingAuthDialog) {
            _pendingAuthDialog->showLoginError(error);
            ErrorHandler::instance().showError("Ошибка входа", error);
        }
    });
    connect(_apiClient.get(), &ApiClient::registerSuccess, this, [this]() {
        if (_pendingAuthDialog) {
            ErrorHandler::instance().showInfo("Успех", "Регистрация успешна");
            _pendingAuthDialog->accept();
        }
    });
    connect(_apiClient.get(), &ApiClient::registerError, this, [this](const QString &error) {
        if (_pendingAuthDialog) {
            _pendingAuthDialog->showRegisterError(error);
            ErrorHandler::instance().showError("Ошибка регистрации", error);
        }
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

    qDebug() << "updateAuthUI";
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

void MainWidget::onLoginSuccess(const QString &token)
{
    qDebug() << "SHITloginSuccess";
    _authManager->login(token, _apiClient->getUsername(), _authManager->isRememberMeEnabled());
    _localStorage->setCurrentUser(_authManager->getUsername());
    qDebug() << "SHITSTARTAFTERSUCCESS";
    _syncManager->startUserSync();
}

void MainWidget::onSyncCompletedAuth()
{
    if (_pendingAuthType == "login") {
        _isGuestMode = false;
        initApplication();
        if (_pendingAuthDialog) _pendingAuthDialog->accept();
        this->show();
    }
}

void MainWidget::onSyncErrorAuth(const QString &err)
{
    if (_pendingAuthType == "login") {
        if (_pendingAuthDialog) {
            ErrorHandler::instance().showError("Ошибка синхронизации", err);
            _pendingAuthDialog->accept();
        }
    }
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
    // Удаляем старый layout, если он есть
    if (layout()) {
        QLayout *oldLayout = layout();
        delete oldLayout;
    }

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
    _localStorage->clearUserData();
    _workspaceController->loadWorkspaces();
    updateWorkspaceList();
    updateAuthUI();
    _leftPanel->refreshWorkspaceList();
}

void MainWidget::updateWorkspaceList()
{
    if (auto leftPanel = findChild<LeftPanel *>()) {
        leftPanel->updateWorkspaceList();
    }
}

void MainWidget::onSyncCompleted()
{
    // Заглушка для слота, чтобы не было ошибки линковки
}

void MainWidget::onSyncError(const QString &error)
{
    // Заглушка для слота, чтобы не было ошибки линковки
}

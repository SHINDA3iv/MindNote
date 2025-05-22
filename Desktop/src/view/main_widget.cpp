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
#include <QDebug>

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

    if (!_authManager->isAuthenticated()) {
        showAuthDialog();
    } else {
        initApplication();
    }

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
    connect(_workspaceController.get(), &WorkspaceController::workspaceRemoved, _editorWidget.get(),
            &EditorWidget::onWorkspaceRemoved);

    // initUI();
    // initConnections();
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
    auto authDialog = new AuthDialog(this);
    authDialog->setWindowTitle("Авторизация");
    authDialog->setWindowModality(Qt::ApplicationModal);

    connect(authDialog, &AuthDialog::loginRequested, _apiClient.get(), &ApiClient::login);
    connect(authDialog, &AuthDialog::registerRequested, _apiClient.get(), &ApiClient::registerUser);
    connect(authDialog, &AuthDialog::guestLoginRequested, this, [this]() {
        _isGuestMode = true;
        initApplication();
    });

    connect(_apiClient.get(), &ApiClient::loginSuccess, this, [authDialog](const QString &token) {
        authDialog->accept();
    });
    connect(_apiClient.get(), &ApiClient::loginError, this, [authDialog](const QString &error) {
        QMessageBox::warning(authDialog, "Ошибка входа", error);
    });
    connect(_apiClient.get(), &ApiClient::registerSuccess, this, [authDialog]() {
        QMessageBox::information(authDialog, "Успех", "Регистрация успешна");
    });
    connect(_apiClient.get(), &ApiClient::registerError, this, [authDialog](const QString &error) {
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
    showAuthDialog();
}

void MainWidget::onLogout()
{
    _authManager->logout();
    updateAuthUI();
}

void MainWidget::initConnections()
{
    connect(_leftPanel.get(), &LeftPanel::workspaceSelected, _editorWidget.get(),
            &EditorWidget::setCurrentWorkspace);
    connect(_leftPanel.get(), &LeftPanel::subWorkspaceSelected, _editorWidget.get(),
            &EditorWidget::setCurrentWorkspace);

    // Auth connections
    connect(_authManager.get(), &AuthManager::authStateChanged, this,
            &MainWidget::onAuthStateChanged);
    connect(_authManager.get(), &AuthManager::loginRequested, this, &MainWidget::onLoginRequested);
    connect(_authManager.get(), &AuthManager::logoutRequested, this, &MainWidget::onLogout);
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

    // Update status message
    if (isAuthenticated) {
        emit statusMessage(tr("Logged in as: %1").arg(username));
    } else {
        emit statusMessage(tr("Not logged in"));
    }
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

    qDebug() << "Saving current workspace:" << currentWorkspace->getName();

    if (_workspaceController) {
        _workspaceController->saveWorkspaces();
        _hasUnsavedChanges = false;
        emit statusMessage("Workspace saved: " + currentWorkspace->getName());
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

void MainWidget::syncWorkspaces()
{
    // TODO: Implement workspace synchronization
    emit statusMessage("Workspaces synchronized");
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

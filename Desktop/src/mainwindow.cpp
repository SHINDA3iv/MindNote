#include "mainwindow.h"
#include "theme/theme_manager.h"
#include "settings/settings_manager.h"
#include "view/auth_dialog.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QCloseEvent>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    _mainWidget = std::make_unique<MainWidget>(this);
    setCentralWidget(_mainWidget.get());
    centralWidget()->setContentsMargins(5, 10, 10, 5);

    createMenus();
    restoreWindowState();
    ThemeManager::instance().applyTheme();

    statusBar()->showMessage("Ready");
    connect(_mainWidget.get(), &MainWidget::statusMessage, this, [this](const QString &msg) {
        statusBar()->showMessage(msg);
    });

    showMaximized();
}

MainWindow::~MainWindow()
{
    saveWindowState();
}

void MainWindow::createMenus()
{
    // File menu
    QMenu *fileMenu = menuBar()->addMenu("Файл");

    QAction *newAction = fileMenu->addAction("Новый");
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, _mainWidget.get(), &MainWidget::createNewWorkspace);

    QAction *createWorkspaceAction = fileMenu->addAction("Создать рабочее пространство");
    connect(createWorkspaceAction, &QAction::triggered, [this]() {
        if (_mainWidget) {
            auto leftPanel = _mainWidget->findChild<LeftPanel *>();
            if (leftPanel) {
                QMetaObject::invokeMethod(leftPanel, "onCreateWorkspace");
            }
        }
    });

    QAction *openAction = fileMenu->addAction("Открыть");
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, _mainWidget.get(), &MainWidget::openWorkspace);

    fileMenu->addSeparator();

    QAction *saveAction = fileMenu->addAction("Сохранить");
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, _mainWidget.get(), &MainWidget::saveCurrentWorkspace);

    QAction *saveAsAction = fileMenu->addAction("Сохранить как...");
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, _mainWidget.get(), &MainWidget::saveWorkspaceAs);

    fileMenu->addSeparator();

    QAction *exitAction = fileMenu->addAction("Выход");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::close);

    // Edit menu
    QMenu *editMenu = menuBar()->addMenu("Правка");

    QAction *undoAction = editMenu->addAction("Отменить");
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered, _mainWidget.get(), &MainWidget::undo);

    QAction *redoAction = editMenu->addAction("Повторить");
    redoAction->setShortcut(QKeySequence::Redo);
    connect(redoAction, &QAction::triggered, _mainWidget.get(), &MainWidget::redo);

    editMenu->addSeparator();

    QAction *cutAction = editMenu->addAction("Вырезать");
    cutAction->setShortcut(QKeySequence::Cut);
    connect(cutAction, &QAction::triggered, _mainWidget.get(), &MainWidget::cut);

    QAction *copyAction = editMenu->addAction("Копировать");
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, _mainWidget.get(), &MainWidget::copy);

    QAction *pasteAction = editMenu->addAction("Вставить");
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction, &QAction::triggered, _mainWidget.get(), &MainWidget::paste);

    // View menu
    QMenu *viewMenu = menuBar()->addMenu("Вид");

    QAction *zoomInAction = viewMenu->addAction("Увеличить");
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    connect(zoomInAction, &QAction::triggered, _mainWidget.get(), &MainWidget::zoomIn);

    QAction *zoomOutAction = viewMenu->addAction("Уменьшить");
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    connect(zoomOutAction, &QAction::triggered, _mainWidget.get(), &MainWidget::zoomOut);

    QAction *zoomResetAction = viewMenu->addAction("Сбросить масштаб");
    zoomResetAction->setShortcut(QKeySequence("Ctrl+0"));
    connect(zoomResetAction, &QAction::triggered, _mainWidget.get(), &MainWidget::zoomReset);

    viewMenu->addSeparator();

    QAction *toggleSidebarAction = viewMenu->addAction("Показать/скрыть боковую панель");
    toggleSidebarAction->setShortcut(QKeySequence("Ctrl+B"));
    connect(toggleSidebarAction, &QAction::triggered, _mainWidget.get(),
            &MainWidget::toggleSidebar);

    // Tools menu
    QMenu *toolsMenu = menuBar()->addMenu("Инструменты");

    QAction *syncAction = toolsMenu->addAction("Синхронизировать");
    syncAction->setShortcut(QKeySequence("Ctrl+S"));
    connect(syncAction, &QAction::triggered, _mainWidget.get(), &MainWidget::syncWorkspaces);

    QAction *settingsAction = toolsMenu->addAction("Настройки");
    settingsAction->setShortcut(QKeySequence("Ctrl+,"));
    connect(settingsAction, &QAction::triggered, this, &MainWindow::showSettings);

    // Help menu
    QMenu *helpMenu = menuBar()->addMenu("Справка");

    QAction *aboutAction = helpMenu->addAction("О программе");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);

    // Auth menu (right-aligned)
    // _authMenu = menuBar()->addMenu("Аутентификация");
    // menuBar()->setCornerWidget(_authMenu, Qt::TopRightCorner);

    // _userAction = new QAction("Гость", this);
    // _userAction->setEnabled(false);
    // _authMenu->addAction(_userAction);

    // _loginAction = new QAction("Войти", this);
    // _loginAction->setIcon(QIcon(":/icons/login.png"));
    // connect(_loginAction, &QAction::triggered, this, &MainWindow::onLoginRequested);
    // _authMenu->addAction(_loginAction);

    // _logoutAction = new QAction("Выйти", this);
    // _logoutAction->setIcon(QIcon(":/icons/logout.png"));
    // _logoutAction->setVisible(false);
    // connect(_logoutAction, &QAction::triggered, this, &MainWindow::onLogoutRequested);
    // _authMenu->addAction(_logoutAction);

    // // Connect to MainWidget's auth state changes
    // connect(_mainWidget.get(), &MainWidget::authStateChanged, this, &MainWindow::updateAuthMenu);
}

void MainWindow::updateAuthMenu()
{
    bool isAuthenticated = _mainWidget->isAuthenticated();
    QString username = _mainWidget->getUsername();

    _userAction->setText(isAuthenticated ? username : "Гость");
    _loginAction->setVisible(!isAuthenticated);
    _logoutAction->setVisible(isAuthenticated);
}

void MainWindow::onLoginRequested()
{
    _mainWidget->showAuthDialog();
}

void MainWindow::onLogoutRequested()
{
    if (QMessageBox::question(this, "Подтверждение выхода",
                              "Вы уверены, что хотите выйти из аккаунта?",
                              QMessageBox::Yes | QMessageBox::No)
        == QMessageBox::Yes) {
        _mainWidget->logout();
    }
}

void MainWindow::showSettings()
{
    SettingsDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        ThemeManager::instance().applyTheme();
    }
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, "О программе",
                       "<h3>MindNote</h3>"
                       "<p>Версия 1.0.0</p>"
                       "<p>MindNote - это приложение для создания и управления заметками и "
                       "рабочими пространствами.</p>"
                       "<p>© 2024 MindNote. Все права защищены.</p>");
}

void MainWindow::restoreWindowState()
{
    auto &settings = SettingsManager::instance();
    restoreGeometry(settings.windowGeometry());
    restoreState(settings.windowState());
}

void MainWindow::saveWindowState()
{
    auto &settings = SettingsManager::instance();
    settings.setWindowGeometry(saveGeometry());
    settings.setWindowState(saveState());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (_mainWidget->hasUnsavedChanges()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
         this, "Несохраненные изменения",
         "У вас есть несохраненные изменения. Хотите сохранить их перед выходом?",
         QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (reply == QMessageBox::Save) {
            if (_mainWidget->saveCurrentWorkspace()) {
                event->accept();
            } else {
                event->ignore();
            }
        } else if (reply == QMessageBox::Discard) {
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

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
#include <QSizePolicy>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidgetItem>
#include <QVariant>
#include <QDebug>

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

    // Connect workspace controller signals
    connect(_mainWidget.get(), &MainWidget::workspaceAdded, this, &MainWindow::updateWorkspaceList);
    connect(_mainWidget.get(), &MainWidget::workspaceRemoved, this, &MainWindow::updateWorkspaceList);

    // Подключаем к изменениям состояния аутентификации
    connect(_mainWidget.get(), &MainWidget::authStateChanged, this, &MainWindow::updateAuthMenu);
    
    // Update auth UI with initial state
    updateAuthMenu();
}

MainWindow::~MainWindow()
{
    saveWindowState();
}

void MainWindow::createMenus()
{
    // File menu
    QMenu *fileMenu = menuBar()->addMenu("Файл");

    QAction *createWorkspaceAction = fileMenu->addAction(QIcon(":/icons/add.png"), "Создать рабочее пространство");
    connect(createWorkspaceAction, &QAction::triggered, [this]() {
        if (_mainWidget) {
            auto leftPanel = _mainWidget->findChild<LeftPanel *>();
            if (leftPanel) {
                QMetaObject::invokeMethod(leftPanel, "onCreateWorkspace");
            }
        }
    });

    QAction *openAction = fileMenu->addAction(QIcon(":/icons/open.png"), "Открыть");
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, _mainWidget.get(), &MainWidget::openWorkspace);

    fileMenu->addSeparator();

    QAction *saveAction = fileMenu->addAction(QIcon(":/icons/save.png"), "Сохранить");
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, _mainWidget.get(), &MainWidget::saveCurrentWorkspace);

    QAction *saveAsAction = fileMenu->addAction(QIcon(":/icons/save-as.png"), "Сохранить как...");
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, _mainWidget.get(), &MainWidget::saveWorkspaceAs);

    fileMenu->addSeparator();

    QAction *exitAction = fileMenu->addAction(QIcon(":/icons/shutdown.png"), "Выход");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::close);

    // Edit menu
    QMenu *editMenu = menuBar()->addMenu("Правка");

    QAction *undoAction = editMenu->addAction(QIcon(":/icons/undo.png"), "Отменить");
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered, _mainWidget.get(), &MainWidget::undo);

    QAction *redoAction = editMenu->addAction(QIcon(":/icons/redo.png"), "Повторить");
    redoAction->setShortcut(QKeySequence::Redo);
    connect(redoAction, &QAction::triggered, _mainWidget.get(), &MainWidget::redo);

    editMenu->addSeparator();

    QAction *cutAction = editMenu->addAction(QIcon(":/icons/cut.png"), "Вырезать");
    cutAction->setShortcut(QKeySequence::Cut);
    connect(cutAction, &QAction::triggered, _mainWidget.get(), &MainWidget::cut);

    QAction *copyAction = editMenu->addAction(QIcon(":/icons/copy.png"), "Копировать");
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, _mainWidget.get(), &MainWidget::copy);

    QAction *pasteAction = editMenu->addAction(QIcon(":/icons/paste.png"), "Вставить");
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction, &QAction::triggered, _mainWidget.get(), &MainWidget::paste);

    // View menu
    QMenu *viewMenu = menuBar()->addMenu("Вид");

    QAction *zoomInAction = viewMenu->addAction(QIcon(":/icons/edit.png"), "Увеличить");
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    connect(zoomInAction, &QAction::triggered, _mainWidget.get(), &MainWidget::zoomIn);

    QAction *zoomOutAction = viewMenu->addAction(QIcon(":/icons/edit.png"), "Уменьшить");
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    connect(zoomOutAction, &QAction::triggered, _mainWidget.get(), &MainWidget::zoomOut);

    QAction *zoomResetAction = viewMenu->addAction(QIcon(":/icons/edit.png"), "Сбросить масштаб");
    zoomResetAction->setShortcut(QKeySequence("Ctrl+0"));
    connect(zoomResetAction, &QAction::triggered, _mainWidget.get(), &MainWidget::zoomReset);

    viewMenu->addSeparator();

    QAction *toggleSidebarAction = viewMenu->addAction(QIcon(":/icons/edit.png"), "Показать/скрыть боковую панель");
    toggleSidebarAction->setShortcut(QKeySequence("Ctrl+B"));
    connect(toggleSidebarAction, &QAction::triggered, _mainWidget.get(),
            &MainWidget::toggleSidebar);

    // Tools menu
    QMenu *toolsMenu = menuBar()->addMenu("Инструменты");

    QAction *syncAction = toolsMenu->addAction(QIcon(":/icons/sync.png"), "Синхронизировать");
    syncAction->setShortcut(QKeySequence("Ctrl+S"));
    connect(syncAction, &QAction::triggered, _mainWidget.get(), &MainWidget::syncWorkspaces);

    QAction *settingsAction = toolsMenu->addAction(QIcon(":/icons/settings.png"), "Настройки");
    settingsAction->setShortcut(QKeySequence("Ctrl+,"));
    connect(settingsAction, &QAction::triggered, this, &MainWindow::showSettings);

    // Help menu
    QMenu *helpMenu = menuBar()->addMenu("Справка");

    QAction *aboutAction = helpMenu->addAction(QIcon(":/icons/about.png"), "О программе");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);

    // Auth elements (right-aligned)
    QWidget* authWidget = new QWidget(this);
    QHBoxLayout* authLayout = new QHBoxLayout(authWidget);
    authLayout->setContentsMargins(8, 0, 8, 0);
    authLayout->setSpacing(8);

    _userLabel = new QLabel("Гость", this);
    _userLabel->setStyleSheet("padding: 4px 8px;");
    authLayout->addWidget(_userLabel);

    _loginButton = new QPushButton("Войти", this);
    _loginButton->setIcon(QIcon(":/icons/login.png"));
    _loginButton->setStyleSheet("padding: 4px 8px;");
    connect(_loginButton, &QPushButton::clicked, this, &MainWindow::onLoginRequested);
    authLayout->addWidget(_loginButton);

    _logoutButton = new QPushButton("Выйти", this);
    _logoutButton->setIcon(QIcon(":/icons/logout.png"));
    _logoutButton->setStyleSheet("padding: 4px 8px;");
    _logoutButton->setVisible(false);
    connect(_logoutButton, &QPushButton::clicked, this, &MainWindow::onLogoutRequested);
    authLayout->addWidget(_logoutButton);

    // Добавляем виджет аутентификации в правый угол
    QWidget* spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    menuBar()->setCornerWidget(spacer, Qt::TopLeftCorner);
    menuBar()->setCornerWidget(authWidget, Qt::TopRightCorner);
}

void MainWindow::updateAuthMenu()
{
    bool isAuthenticated = _mainWidget->isAuthenticated();
    QString username = _mainWidget->getUsername();

    qDebug() << "Updating auth menu - isAuthenticated:" << isAuthenticated << "username:" << username;

    _userLabel->setText(isAuthenticated ? username : "Гость");
    _loginButton->setVisible(!isAuthenticated);
    _logoutButton->setVisible(isAuthenticated);

    // Force update of the menu bar
    menuBar()->update();
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

void MainWindow::updateWorkspaceList()
{
    if (_mainWidget) {
        _mainWidget->updateWorkspaceList();
    }
}

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "view/main_widget.h"
#include "view/settings_dialog.h"

#include <QMainWindow>
#include <QCloseEvent>
#include <QMenu>
#include <QAction>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void showSettings();
    void showAbout();
    void updateAuthMenu();
    void onLoginRequested();
    void onLogoutRequested();

private:
    void createMenus();
    void restoreWindowState();
    void saveWindowState();

    std::unique_ptr<MainWidget> _mainWidget;
    
    // Auth menu items
    QMenu* _authMenu;
    QAction* _loginAction;
    QAction* _logoutAction;
    QAction* _userAction;
};

#endif // MAINWINDOW_H

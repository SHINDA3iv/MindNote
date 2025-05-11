#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "view/main_widget.h"
#include "view/settings_dialog.h"

#include <QMainWindow>
#include <QCloseEvent>

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

private:
    void createMenus();
    void restoreWindowState();
    void saveWindowState();

    std::unique_ptr<MainWidget> _mainWidget;
};

#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "view/main_widget.h"

#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

private:
    std::unique_ptr<MainWidget> _mainWidget;
};
#endif // MAINWINDOW_H

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    _mainWidget = std::make_unique<MainWidget>(this);
    setCentralWidget(_mainWidget.get());
    showMaximized();
}

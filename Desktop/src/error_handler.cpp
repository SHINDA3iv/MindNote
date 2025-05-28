#include "error_handler.h"
#include <QApplication>
#include <QWidget>

ErrorHandler& ErrorHandler::instance()
{
    static ErrorHandler instance;
    return instance;
}

ErrorHandler::ErrorHandler(QObject *parent) : QObject(parent)
{
}

void ErrorHandler::showError(const QString& title, const QString& message)
{
    QMessageBox::critical(QApplication::activeWindow(), title, message);
}

void ErrorHandler::showWarning(const QString& title, const QString& message)
{
    QMessageBox::warning(QApplication::activeWindow(), title, message);
}

void ErrorHandler::showInfo(const QString& title, const QString& message)
{
    QMessageBox::information(QApplication::activeWindow(), title, message);
} 
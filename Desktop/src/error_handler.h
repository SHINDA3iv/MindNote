#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <QObject>
#include <QString>
#include <QMessageBox>

class ErrorHandler : public QObject
{
    Q_OBJECT

public:
    static ErrorHandler& instance();

    void showError(const QString& title, const QString& message);
    void showWarning(const QString& title, const QString& message);
    void showInfo(const QString& title, const QString& message);

private:
    explicit ErrorHandler(QObject *parent = nullptr);
    ~ErrorHandler() = default;
    ErrorHandler(const ErrorHandler&) = delete;
    ErrorHandler& operator=(const ErrorHandler&) = delete;
};

#endif // ERROR_HANDLER_H 
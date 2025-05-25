#pragma once

#include <QDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <QMap>

class QTableWidget;
class QCheckBox;
class QRadioButton;

class UserSyncDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UserSyncDialog(const QJsonObject &diff, QWidget *parent = nullptr);

    QJsonArray getResolve() const;
    QJsonArray getNewWorkspaces() const;

private:
    void setupUI(const QJsonObject &diff);
    void buildNewTable(const QJsonArray &newArr);
    void buildConflictsTable(const QJsonArray &conflictsArr);

    QTableWidget *newTable = nullptr;
    QTableWidget *conflictsTable = nullptr;
    QVector<QCheckBox*> newCheckBoxes;
    // Используем QPair<QRadioButton*, QRadioButton*> для хранения пары радиокнопок
    QVector<QPair<QRadioButton*, QRadioButton*>> conflictRadioButtons;
    QVector<QJsonObject> newWorkspacesData;
    QVector<QJsonObject> conflictLocalData;
    QVector<QJsonObject> conflictServerData;
}; 
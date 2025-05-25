#include "user_sync_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QCheckBox>
#include <QRadioButton>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

UserSyncDialog::UserSyncDialog(const QJsonObject &diff, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Синхронизация рабочих пространств");
    resize(700, 500);
    setupUI(diff);
}

void UserSyncDialog::setupUI(const QJsonObject &diff)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QLabel *infoLabel = new QLabel("Выберите, какие рабочие пространства добавить и какие версии оставить при конфликте:", this);
    mainLayout->addWidget(infoLabel);

    if (diff.contains("new")) {
        buildNewTable(diff["new"].toArray());
        if (newTable) mainLayout->addWidget(newTable);
    }
    if (diff.contains("conflicts")) {
        buildConflictsTable(diff["conflicts"].toArray());
        if (conflictsTable) mainLayout->addWidget(conflictsTable);
    }

    QHBoxLayout *btnLayout = new QHBoxLayout;
    QPushButton *okBtn = new QPushButton("OK", this);
    QPushButton *cancelBtn = new QPushButton("Отмена", this);
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);
}

void UserSyncDialog::buildNewTable(const QJsonArray &newArr)
{
    if (newArr.isEmpty()) return;
    newTable = new QTableWidget(newArr.size(), 2, this);
    newTable->setHorizontalHeaderLabels({"Добавить", "Название"});
    newTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    newCheckBoxes.clear();
    newWorkspacesData.clear();
    for (int i = 0; i < newArr.size(); ++i) {
        QJsonObject ws = newArr[i].toObject();
        QString title = ws["title"].toString();
        QCheckBox *cb = new QCheckBox(this);
        cb->setChecked(true);
        newTable->setCellWidget(i, 0, cb);
        newTable->setItem(i, 1, new QTableWidgetItem(title));
        newCheckBoxes.append(cb);
        newWorkspacesData.append(ws);
    }
    newTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void UserSyncDialog::buildConflictsTable(const QJsonArray &conflictsArr)
{
    if (conflictsArr.isEmpty()) return;
    conflictsTable = new QTableWidget(conflictsArr.size(), 4, this);
    conflictsTable->setHorizontalHeaderLabels({"Локальная версия", "Серверная версия", "Название", "Различия"});
    conflictsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    conflictRadioButtons.clear();
    conflictLocalData.clear();
    conflictServerData.clear();
    for (int i = 0; i < conflictsArr.size(); ++i) {
        QJsonObject conflict = conflictsArr[i].toObject();
        QString title = conflict["title"].toString();
        QRadioButton *localBtn = new QRadioButton("Локальная", this);
        QRadioButton *serverBtn = new QRadioButton("Серверная", this);
        localBtn->setChecked(true);
        QWidget *localWidget = new QWidget(this);
        QHBoxLayout *localLayout = new QHBoxLayout(localWidget);
        localLayout->addWidget(localBtn);
        localLayout->setContentsMargins(0,0,0,0);
        localWidget->setLayout(localLayout);
        QWidget *serverWidget = new QWidget(this);
        QHBoxLayout *serverLayout = new QHBoxLayout(serverWidget);
        serverLayout->addWidget(serverBtn);
        serverLayout->setContentsMargins(0,0,0,0);
        serverWidget->setLayout(serverLayout);
        conflictsTable->setCellWidget(i, 0, localWidget);
        conflictsTable->setCellWidget(i, 1, serverWidget);
        conflictsTable->setItem(i, 2, new QTableWidgetItem(title));
        QPushButton *diffBtn = new QPushButton("Показать различия", this);
        connect(diffBtn, &QPushButton::clicked, this, [=]() {
            QMessageBox::information(this, "Diff", "Показ различий между версиями\n(реализуй по желанию)");
        });
        conflictsTable->setCellWidget(i, 3, diffBtn);
        conflictRadioButtons.append(QPair<QRadioButton*, QRadioButton*>(localBtn, serverBtn));
        conflictLocalData.append(conflict["local"].toObject());
        conflictServerData.append(conflict["server"].toObject());
    }
    conflictsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

QJsonArray UserSyncDialog::getResolve() const
{
    QJsonArray arr;
    for (int i = 0; i < conflictRadioButtons.size(); ++i) {
        QJsonObject obj;
        obj["title"] = conflictLocalData[i]["title"].toString();
        if (conflictRadioButtons[i].first->isChecked()) {
            obj["use"] = "local";
            obj["data"] = conflictLocalData[i];
        } else {
            obj["use"] = "server";
        }
        arr.append(obj);
    }
    return arr;
}

QJsonArray UserSyncDialog::getNewWorkspaces() const
{
    QJsonArray arr;
    for (int i = 0; i < newCheckBoxes.size(); ++i) {
        if (newCheckBoxes[i]->isChecked()) {
            arr.append(newWorkspacesData[i]);
        }
    }
    return arr;
} 
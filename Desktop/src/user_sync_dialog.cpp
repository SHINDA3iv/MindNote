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
#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QApplication>
#include <QIcon>

UserSyncDialog::UserSyncDialog(const QJsonObject &diff, QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Синхронизация рабочих пространств");
    resize(700, 500);
    // Если нет новых и конфликтных — не показываем диалог
    bool hasNew = diff.contains("new") && !diff["new"].toArray().isEmpty();
    bool hasConflicts = diff.contains("conflicts") && !diff["conflicts"].toArray().isEmpty();
    if (!hasNew && !hasConflicts) {
        accept();
        return;
    }
    qDebug() << "SHIT#";
    // Применяем общий стиль приложения
    setStyleSheet(qApp->styleSheet());
    setupUI(diff);
}

void UserSyncDialog::setupUI(const QJsonObject &diff)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(18);
    // --- Иконка + текст ---
    QHBoxLayout *infoLayout = new QHBoxLayout;
    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(QIcon(":/resources/icons/sync.png").pixmap(28, 28));
    QLabel *infoLabel = new QLabel("Выберите, какие рабочие пространства добавить и какие версии оставить при конфликте:", this);
    infoLabel->setStyleSheet("font-size: 16px; font-weight: 500; margin-bottom: 12px;");
    infoLayout->addWidget(iconLabel);
    infoLayout->addSpacing(8);
    infoLayout->addWidget(infoLabel);
    infoLayout->addStretch();
    mainLayout->addLayout(infoLayout);
    // --- конец блока ---

    if (diff.contains("new")) {
        buildNewTable(diff["new"].toArray());
        if (newTable)
            mainLayout->addWidget(newTable);
    }
    if (diff.contains("conflicts")) {
        buildConflictsTable(diff["conflicts"].toArray());
        if (conflictsTable)
            mainLayout->addWidget(conflictsTable);
    }

    QHBoxLayout *btnLayout = new QHBoxLayout;
    QPushButton *okBtn = new QPushButton(QIcon(":/resources/icons/sync.png"), "OK", this);
    QPushButton *cancelBtn = new QPushButton(QIcon(":/resources/icons/close.png"), "Отмена", this);
    okBtn->setStyleSheet(
     "padding: 6px 18px; border-radius: 8px; background:#4caf50; color:white; font-weight:500;");
    cancelBtn->setStyleSheet(
     "padding: 6px 18px; border-radius: 8px; background:#e53935; color:white; font-weight:500;");
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);
}

void UserSyncDialog::buildNewTable(const QJsonArray &newArr)
{
    if (newArr.isEmpty())
        return;
    newTable = new QTableWidget(newArr.size(), 2, this);
    newTable->setHorizontalHeaderLabels({ "Добавить", "Название" });
    newTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    newTable->setStyleSheet(R"(
        QTableWidget {
            background: #fafbfc;
            border-radius: 10px;
            border: 1px solid #e0e0e0;
            font-size: 15px;
        }
        QHeaderView::section {
            background: #f5f5f5;
            font-weight: 500;
            border: none;
            border-bottom: 1px solid #e0e0e0;
            padding: 6px;
        }
        QTableWidget::item {
            padding: 8px;
        }
        QTableCornerButton::section {
            background: #f5f5f5;
            border: none;
        }
    )");
    newCheckBoxes.clear();
    newWorkspacesData.clear();
    for (int i = 0; i < newArr.size(); ++i) {
        QJsonObject ws = newArr[i].toObject();
        QString title = ws["title"].toString();
        QCheckBox *cb = new QCheckBox(this);
        cb->setChecked(true);
        cb->setStyleSheet("margin-left:8px;margin-right:8px;");
        newTable->setCellWidget(i, 0, cb);
        QTableWidgetItem *item =
         new QTableWidgetItem(QIcon(":/resources/icons/workspace.png"), title);
        newTable->setItem(i, 1, item);
        newCheckBoxes.append(cb);
        newWorkspacesData.append(ws);
    }
    newTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    newTable->setSelectionMode(QAbstractItemView::NoSelection);
    newTable->setShowGrid(false);
    newTable->verticalHeader()->setVisible(false);
}

void UserSyncDialog::buildConflictsTable(const QJsonArray &conflictsArr)
{
    if (conflictsArr.isEmpty()) return;
    conflictsTable = new QTableWidget(conflictsArr.size(), 3, this);
    conflictsTable->setHorizontalHeaderLabels({"Локальная версия", "Серверная версия", "Название"});
    conflictsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    conflictsTable->setStyleSheet(R"(
        QTableWidget {
            background: #fafbfc;
            border-radius: 10px;
            border: 1px solid #e0e0e0;
            font-size: 15px;
        }
        QHeaderView::section {
            background: #f5f5f5;
            font-weight: 500;
            border: none;
            border-bottom: 1px solid #e0e0e0;
            padding: 6px;
        }
        QTableWidget::item {
            padding: 8px;
        }
        QTableCornerButton::section {
            background: #f5f5f5;
            border: none;
        }
    )");
    conflictRadioButtons.clear();
    conflictLocalData.clear();
    conflictServerData.clear();
    for (int i = 0; i < conflictsArr.size(); ++i) {
        QJsonObject conflict = conflictsArr[i].toObject();
        QString title = conflict["title"].toString();
        QRadioButton *localBtn = new QRadioButton("Локальная", this);
        QRadioButton *serverBtn = new QRadioButton("Серверная", this);
        localBtn->setChecked(true);
        localBtn->setStyleSheet("margin-left:8px;margin-right:8px;");
        serverBtn->setStyleSheet("margin-left:8px;margin-right:8px;");
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
        QTableWidgetItem *item = new QTableWidgetItem(QIcon(":/resources/icons/workspace.png"), title);
        conflictsTable->setItem(i, 2, item);
        conflictRadioButtons.append(QPair<QRadioButton*, QRadioButton*>(localBtn, serverBtn));
        conflictLocalData.append(conflict["local"].toObject());
        conflictServerData.append(conflict["server"].toObject());
    }
    conflictsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    conflictsTable->setSelectionMode(QAbstractItemView::NoSelection);
    conflictsTable->setShowGrid(false);
    conflictsTable->verticalHeader()->setVisible(false);
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

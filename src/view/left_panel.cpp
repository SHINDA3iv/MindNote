#include "left_panel.h"
#include <qpushbutton.h>

#include <QInputDialog>
#include <QMenu>
#include <QIcon>
#include <QFileDialog>
#include <QMessageBox>

LeftPanel::LeftPanel(QWidget *parent) :
    QWidget(parent),
    _workspaceList(new QListWidget(this)),
    _createWorkspaceButton(new QToolButton(this))
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    _createWorkspaceButton->setIcon(QIcon::fromTheme("document-new"));
    _createWorkspaceButton->setIconSize(QSize(16, 16));
    _createWorkspaceButton->setToolTip(tr("Создать новое пространство"));
    _createWorkspaceButton->setFixedSize(32, 32);

    layout->addWidget(_createWorkspaceButton);
    layout->addWidget(_workspaceList);
    setLayout(layout);

    connect(_workspaceList, &QListWidget::itemClicked, this, &LeftPanel::onWorkspaceClicked);
    connect(_createWorkspaceButton, &QToolButton::clicked, this, &LeftPanel::onCreateWorkspace);

    _workspaceList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_workspaceList, &QListWidget::customContextMenuRequested, this,
            &LeftPanel::showContextMenu);
}

void LeftPanel::setWorkspaceController(WorkspaceController *controller)
{
    _workspaceController = controller;
    refreshWorkspaceList();
}

void LeftPanel::onCreateWorkspace()
{
    if (!_workspaceController)
        return;

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(tr("Создание нового пространства"));

    QVBoxLayout *dialogLayout = new QVBoxLayout(dialog);

    QLineEdit *nameEdit = new QLineEdit(dialog);
    nameEdit->setPlaceholderText(tr("Введите имя нового пространства"));
    dialogLayout->addWidget(nameEdit);

    QPushButton *chooseIconButton = new QPushButton(tr("Выбрать иконку"), dialog);
    dialogLayout->addWidget(chooseIconButton);

    // Выравнивание для QLabel с иконкой
    QLabel *iconLabel = new QLabel(dialog);
    iconLabel->setPixmap(QPixmap());
    iconLabel->setAlignment(Qt::AlignCenter); // Выравнивание иконки по центру
    dialogLayout->addWidget(iconLabel);

    QIcon selectedIcon;

    connect(chooseIconButton, &QPushButton::clicked, [this, iconLabel, &selectedIcon]() {
        QString iconPath =
         QFileDialog::getOpenFileName(this, tr("Выберите иконку"), QString(),
                                      tr("Image Files (*.png *.jpg *.jpeg *.bmp *.gif)"));
        if (!iconPath.isEmpty()) {
            selectedIcon = QIcon(iconPath);
            iconLabel->setPixmap(selectedIcon.pixmap(64, 64)); // Размер иконки 64x64
        }
    });

    QPushButton *createButton = new QPushButton(tr("Создать пространство"), dialog);
    dialogLayout->addWidget(createButton);

    connect(createButton, &QPushButton::clicked, [this, dialog, nameEdit, &selectedIcon]() {
        QString workspaceName = nameEdit->text();

        if (!workspaceName.isEmpty()) {
            auto existingWorkspaces = _workspaceController->getAllWorkspaces();
            bool nameExists = std::any_of(existingWorkspaces.begin(), existingWorkspaces.end(),
                                          [&workspaceName](Workspace *workspace) {
                                              return workspace->getName() == workspaceName;
                                          });

            if (nameExists) {
                QMessageBox::warning(this, tr("Ошибка"),
                                     tr("Пространство с таким именем уже существует."));
                return;
            }

            Workspace *newWorkspace = _workspaceController->createWorkspace(workspaceName);
            if (!selectedIcon.isNull()) {
                newWorkspace->setIcon(selectedIcon);
            }

            refreshWorkspaceList();
            emit workspaceSelected(newWorkspace);
            dialog->accept();
        }
    });

    dialog->exec();
}

void LeftPanel::onWorkspaceClicked(QListWidgetItem *item)
{
    if (!item || !_workspaceController)
        return;

    Workspace *workspace = static_cast<Workspace *>(item->data(Qt::UserRole).value<void *>());
    if (workspace) {
        emit workspaceSelected(workspace);
    }
}

void LeftPanel::refreshWorkspaceList()
{
    if (!_workspaceController)
        return;

    _workspaceList->clear();

    // Устанавливаем размер иконок
    QSize iconSize(28, 20); // Размер иконок 48x48
    _workspaceList->setIconSize(iconSize);

    auto workspaces = _workspaceController->getAllWorkspaces();
    for (Workspace *workspace : workspaces) {
        QListWidgetItem *item = new QListWidgetItem(workspace->getName(), _workspaceList);

        // Устанавливаем иконку, если она есть
        if (!workspace->getIcon()->pixmap().isNull()) {
            item->setIcon(workspace->getIcon()->pixmap().scaled(iconSize));
        } else {
            item->setIcon(QIcon().pixmap(iconSize));
        }

        // Устанавливаем высоту строки
        item->setSizeHint(QSize(0, 30));

        item->setData(Qt::UserRole, QVariant::fromValue(static_cast<void *>(workspace)));
    }
}

void LeftPanel::showContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = _workspaceList->itemAt(pos);
    if (!item || !_workspaceController)
        return;

    Workspace *workspace = static_cast<Workspace *>(item->data(Qt::UserRole).value<void *>());
    if (!workspace)
        return;

    QMenu contextMenu(this);

    QAction *changeIconAction = contextMenu.addAction("Сменить иконку");
    connect(changeIconAction, &QAction::triggered, [this, workspace]() {
        QString iconPath =
         QFileDialog::getOpenFileName(this, tr("Выберите иконку"), QString(),
                                      tr("Image Files (*.png *.jpg *.jpeg *.bmp *.gif)"));
        if (!iconPath.isEmpty()) {
            workspace->setIcon(QIcon(iconPath));
        }
        refreshWorkspaceList();
    });

    QAction *renameAction = contextMenu.addAction("Сменить название");
    connect(renameAction, &QAction::triggered, [this, workspace, item]() {
        bool ok;
        QString newName =
         QInputDialog::getText(this, "Сменить название пространства",
                               "Новое название:", QLineEdit::Normal, workspace->getName(), &ok);
        if (ok && !newName.isEmpty()) {
            workspace->setName(newName);
            item->setText(newName);
        }
    });

    QAction *deleteAction = contextMenu.addAction("Удалить пространство");
    connect(deleteAction, &QAction::triggered, [this, workspace]() {
        _workspaceController->removeWorkspace(workspace);
        refreshWorkspaceList();
    });

    contextMenu.exec(_workspaceList->viewport()->mapToGlobal(pos));
}

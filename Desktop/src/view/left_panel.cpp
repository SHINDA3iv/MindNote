#include "left_panel.h"
#include <qpushbutton.h>

#include <QInputDialog>
#include <QMenu>
#include <QIcon>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QTreeWidget>
#include <QListWidgetItem>

LeftPanel::LeftPanel(QWidget *parent) :
    QWidget(parent),
    _workspaceTree(new QTreeWidget(this))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(_workspaceTree);
    setLayout(layout);
    _workspaceTree->setHeaderHidden(true);
    connect(_workspaceTree, &QTreeWidget::itemClicked, this, &LeftPanel::onWorkspaceClicked);
    _workspaceTree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_workspaceTree, &QTreeWidget::customContextMenuRequested, this, &LeftPanel::showContextMenu);
}

void LeftPanel::setWorkspaceController(WorkspaceController *controller)
{
    _workspaceController = controller;
    refreshWorkspaceList();
}

void LeftPanel::refreshWorkspaceList()
{
    if (!_workspaceController) return;
    _workspaceTree->clear();
    QSize iconSize(32, 32);
    _workspaceTree->setIconSize(iconSize);
    std::function<void(Workspace*, QTreeWidgetItem*)> addTree = [&](Workspace* ws, QTreeWidgetItem* parentItem) {
        QTreeWidgetItem* item = new QTreeWidgetItem();
        item->setText(0, ws->getName());
        if (!ws->getIcon()->pixmap().isNull()) {
            QPixmap pixmap = ws->getIcon()->pixmap();
            QPixmap paddedPixmap(iconSize.width() + 8, iconSize.height() + 8);
            paddedPixmap.fill(Qt::transparent);
            QPixmap scaledPixmap = pixmap.scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QPainter painter(&paddedPixmap);
            painter.drawPixmap(4, 4, scaledPixmap);
            item->setIcon(0, QIcon(paddedPixmap));
        }
        item->setData(0, Qt::UserRole, QVariant::fromValue(static_cast<void*>(ws)));
        if (parentItem) parentItem->addChild(item);
        else _workspaceTree->addTopLevelItem(item);
        for (Workspace* sub : ws->getSubWorkspaces()) addTree(sub, item);
    };
    for (Workspace* ws : _workspaceController->getRootWorkspaces()) addTree(ws, nullptr);
    _workspaceTree->expandAll();
}

void LeftPanel::onWorkspaceClicked(QTreeWidgetItem *item)
{
    if (!item || !_workspaceController) return;
    Workspace *workspace = static_cast<Workspace *>(item->data(0, Qt::UserRole).value<void *>());
    if (workspace) emit workspaceSelected(workspace);
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

void LeftPanel::onCreateSubWorkspace()
{
    QTreeWidgetItem* item = _workspaceTree->currentItem();
    if (!item || !_workspaceController) return;
    Workspace* parent = static_cast<Workspace*>(item->data(0, Qt::UserRole).value<void*>());
    if (!parent) return;

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(tr("Создание нового подпространства"));

    QVBoxLayout *dialogLayout = new QVBoxLayout(dialog);

    QLineEdit *nameEdit = new QLineEdit(dialog);
    nameEdit->setPlaceholderText(tr("Введите имя нового подпространства"));
    dialogLayout->addWidget(nameEdit);

    QPushButton *chooseIconButton = new QPushButton(tr("Выбрать иконку"), dialog);
    dialogLayout->addWidget(chooseIconButton);

    QLabel *iconLabel = new QLabel(dialog);
    iconLabel->setPixmap(QPixmap());
    iconLabel->setAlignment(Qt::AlignCenter);
    dialogLayout->addWidget(iconLabel);

    QIcon selectedIcon;

    connect(chooseIconButton, &QPushButton::clicked, [this, iconLabel, &selectedIcon]() {
        QString iconPath = QFileDialog::getOpenFileName(this, tr("Выберите иконку"), QString(),
                                                      tr("Image Files (*.png *.jpg *.jpeg *.bmp *.gif)"));
        if (!iconPath.isEmpty()) {
            selectedIcon = QIcon(iconPath);
            iconLabel->setPixmap(selectedIcon.pixmap(64, 64));
        }
    });

    QPushButton *createButton = new QPushButton(tr("Создать подпространство"), dialog);
    dialogLayout->addWidget(createButton);

    connect(createButton, &QPushButton::clicked, [this, dialog, nameEdit, &selectedIcon, parent]() {
        QString subName = nameEdit->text();
        if (subName.isEmpty() || parent->hasSubWorkspaceWithName(subName)) {
            QMessageBox::warning(dialog, tr("Ошибка"), tr("Подпространство с таким именем уже существует."));
            return;
        }

        Workspace *sub = _workspaceController->createSubWorkspace(parent, subName);
        if (sub && !selectedIcon.isNull()) {
            sub->setIcon(selectedIcon);
        }

        refreshWorkspaceList();
        dialog->accept();
    });

    dialog->exec();
    dialog->deleteLater();
}

void LeftPanel::showContextMenu(const QPoint &pos)
{
    QTreeWidgetItem* item = _workspaceTree->itemAt(pos);
    if (!item || !_workspaceController) return;
    Workspace* workspace = static_cast<Workspace*>(item->data(0, Qt::UserRole).value<void*>());
    if (!workspace) return;

    QMenu contextMenu(this);
    
    QAction* addSubAction = contextMenu.addAction(tr("Добавить подпространство"));
    addSubAction->setIcon(QIcon::fromTheme("folder-new"));
    connect(addSubAction, &QAction::triggered, this, &LeftPanel::onCreateSubWorkspace);

    QAction *renameAction = contextMenu.addAction(tr("Переименовать"));
    renameAction->setIcon(QIcon::fromTheme("edit-rename"));

    QAction *changeIconAction = contextMenu.addAction(tr("Изменить иконку"));
    changeIconAction->setIcon(QIcon::fromTheme("image-x-generic"));

    QAction *deleteAction = contextMenu.addAction(tr("Удалить"));
    deleteAction->setIcon(QIcon::fromTheme("edit-delete"));

    connect(renameAction, &QAction::triggered, [this, workspace, item]() {
        bool ok;
        QString newName = QInputDialog::getText(this, tr("Переименовать"), 
                                              tr("Новое название:"), 
                                              QLineEdit::Normal, 
                                              workspace->getName(), 
                                              &ok);
        if (ok && !newName.isEmpty()) {
            workspace->setName(newName);
            item->setText(0, newName);
        }
    });

    connect(changeIconAction, &QAction::triggered, [this, workspace]() {
        QString iconPath = QFileDialog::getOpenFileName(this, tr("Выберите иконку"), "",
                                                      tr("Images (*.png *.jpg *.jpeg *.bmp *.gif)"));
        if (!iconPath.isEmpty()) {
            workspace->setIcon(QIcon(iconPath));
            refreshWorkspaceList();
        }
    });

    connect(deleteAction, &QAction::triggered, [this, workspace]() {
        if (QMessageBox::question(this, tr("Подтверждение"),
                                tr("Вы уверены, что хотите удалить это пространство?"),
                                QMessageBox::Yes | QMessageBox::No)
            == QMessageBox::Yes) {
            _workspaceController->removeWorkspace(workspace);
            refreshWorkspaceList();
        }
    });

    contextMenu.exec(_workspaceTree->viewport()->mapToGlobal(pos));
}

void LeftPanel::updateWorkspaceList()
{
    if (!_workspaceController) return;
    
    _workspaceTree->clear();
    QSize iconSize(32, 32);
    _workspaceTree->setIconSize(iconSize);
    
    // Add root workspaces
    for (Workspace *ws : _workspaceController->getRootWorkspaces()) {
        QTreeWidgetItem *item = new QTreeWidgetItem(_workspaceTree);
        item->setText(0, ws->getName());
        item->setData(0, Qt::UserRole, QVariant::fromValue(ws));
        
        // Set icon
        if (!ws->getIcon()->pixmap().isNull()) {
            QPixmap pixmap = ws->getIcon()->pixmap();
            QPixmap paddedPixmap(iconSize.width() + 8, iconSize.height() + 8);
            paddedPixmap.fill(Qt::transparent);
            QPixmap scaledPixmap = pixmap.scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QPainter painter(&paddedPixmap);
            painter.drawPixmap(4, 4, scaledPixmap);
            item->setIcon(0, QIcon(paddedPixmap));
        } else {
            // Set default icon
            item->setIcon(0, QIcon(":/icons/workspace.png"));
        }
        
        // Add subworkspaces recursively
        addSubWorkspacesToTree(item, ws);
    }
    _workspaceTree->expandAll();
}

void LeftPanel::addSubWorkspacesToTree(QTreeWidgetItem *parentItem, Workspace *parentWorkspace)
{
    for (Workspace *sub : parentWorkspace->getSubWorkspaces()) {
        QTreeWidgetItem *item = new QTreeWidgetItem(parentItem);
        item->setText(0, sub->getName());
        item->setData(0, Qt::UserRole, QVariant::fromValue(sub));
        
        // Set icon
        if (!sub->getIcon()->pixmap().isNull()) {
            QPixmap pixmap = sub->getIcon()->pixmap();
            QSize iconSize = _workspaceTree->iconSize();
            QPixmap paddedPixmap(iconSize.width() + 8, iconSize.height() + 8);
            paddedPixmap.fill(Qt::transparent);
            QPixmap scaledPixmap = pixmap.scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QPainter painter(&paddedPixmap);
            painter.drawPixmap(4, 4, scaledPixmap);
            item->setIcon(0, QIcon(paddedPixmap));
        } else {
            // Set default icon
            item->setIcon(0, QIcon(":/icons/workspace.png"));
        }
        
        // Recursively add subworkspaces
        addSubWorkspacesToTree(item, sub);
    }
}

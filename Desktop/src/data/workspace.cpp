#include "workspace.h"
#include "checkbox_item.h"
#include "file_item.h"
#include "image_item.h"
#include "list_item.h"
#include "nested_workspace_item.h"
#include "text_item.h"
#include "title_item.h"

#include <QScrollArea>
#include <QMenu>
#include <QToolButton>
#include <QIcon>
#include <QFileDialog>

Workspace::Workspace(const QString &name, QWidget *parent) :
    QWidget(parent),
    _workspaceName(name),
    _iconLabel(new QLabel(this))
{
    this->setStyleSheet(R"(
        Workspace {
            background-color: white;
            border: 0px;
        }

        QLabel {
            margin: 5px;
        }

        QScrollArea {
            border: none;
            background: transparent;
        }
    )");

    QVBoxLayout *contentLayout = new QVBoxLayout(this);

    QHBoxLayout *headerLayout = new QHBoxLayout();

    _titleLabel = new QLabel(_workspaceName, this);
    _titleLabel->setAlignment(Qt::AlignCenter);

    QFont titleFont = _titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    _titleLabel->setFont(titleFont);

    _iconLabel->setPixmap(QPixmap());
    _iconLabel->setFixedSize(32, 32);
    _iconLabel->setContentsMargins(0, 0, 0, 0);

    headerLayout->addWidget(_titleLabel);
    headerLayout->addWidget(_iconLabel);

    QToolButton *menuButton = new QToolButton(this);
    menuButton->setIcon(QIcon::fromTheme("menu"));
    // menuButton->setStyleSheet("border: none;");
    menuButton->setToolTip("Добавить элемент");

    QMenu *toolMenu = new QMenu(this);
    toolMenu->addAction(QIcon::fromTheme("text"), "Добавить текст", this, [this]() {
        emit addItemByType("TextItem");
    });
    toolMenu->addAction(QIcon::fromTheme("checkbox"), "Добавить checkbox", this, [this]() {
        emit addItemByType("CheckboxItem");
    });
    toolMenu->addAction(QIcon::fromTheme("list"), "Добавить нумерованный список", this, [this]() {
        emit addItemByType("OrderedListItem");
    });
    toolMenu->addAction(QIcon::fromTheme("list"), "Добавить ненумерованный список", this, [this]() {
        emit addItemByType("UnorderedListItem");
    });
    toolMenu->addAction(QIcon::fromTheme("image"), "Добавить изображение", this, [this]() {
        emit addItemByType("ImageItem");
    });
    toolMenu->addAction(QIcon::fromTheme("file"), "Добавить файл", this, [this]() {
        emit addItemByType("FileItem");
    });

    menuButton->setMenu(toolMenu);
    menuButton->setPopupMode(QToolButton::InstantPopup);

    headerLayout->addWidget(menuButton);

    contentLayout->addLayout(headerLayout);

    _contentWidget = new QWidget(this);
    _layout = new QVBoxLayout(_contentWidget);
    _contentWidget->setLayout(_layout);

    _scrollArea = new QScrollArea(this);
    _scrollArea->setWidget(_contentWidget);
    _scrollArea->setWidgetResizable(true);
    _scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    contentLayout->addWidget(_scrollArea);
    setLayout(contentLayout);
}

QLabel *Workspace::getIcon()
{
    return _iconLabel;
}

void Workspace::setIcon(const QIcon &icon)
{
    _iconLabel->setPixmap(icon.pixmap(32, 32));
}

void Workspace::addItemByType(const QString &type)
{
    AbstractWorkspaceItem *item = nullptr;

    if (type == "TextItem") {
        item = new TextItem("Текст", this);
    } else if (type == "CheckboxItem") {
        item = new CheckboxItem("Задача", this);
    } else if (type == "OrderedListItem") {
        ListItem *list = new ListItem(ListItem::Ordered, this);
        item = list;
    } else if (type == "UnorderedListItem") {
        ListItem *list = new ListItem(ListItem::Unordered, this);
        item = list;
    } else if (type == "ImageItem") {
        QString imagePath = QFileDialog::getOpenFileName(this, "Выберите изображение", "",
                                                         "Images (*.png *.jpg *.jpeg *.bmp *.gif)");
        if (!imagePath.isEmpty()) {
            item = new ImageItem(imagePath, this);
        }
    } else if (type == "FileItem") {
        QString filePath =
         QFileDialog::getOpenFileName(this, "Выберите файл", "", "All Files (*.*)");
        if (!filePath.isEmpty()) {
            item = new FileItem(filePath, this);
        }
    }

    if (item) {
        addItem(item);
    }
}

QString Workspace::getName() const
{
    return _workspaceName;
}

void Workspace::setName(const QString &name)
{
    _workspaceName = name;
    if (_titleLabel) {
        _titleLabel->setText(name);
    }
}

void Workspace::addItem(AbstractWorkspaceItem *item)
{
    _items.append(item);

    if (_spacerItem) {
        _layout->removeItem(_spacerItem);
        delete _spacerItem;
        _spacerItem = nullptr;
    }

    if (auto resizableItem = static_cast<ResizableItem *>(item))
        connect(resizableItem, &ResizableItem::resized, this, &Workspace::updateContentSize);

    item->setMinimumHeight(25);

    connect(item, &AbstractWorkspaceItem::itemDeleted, this, &Workspace::removeItem);
    _layout->addWidget(item);

    updateContentSize();

    _spacerItem = new QSpacerItem(20, _contentWidget->height() - _contentWidget->minimumHeight(),
                                  QSizePolicy::Minimum, QSizePolicy::Fixed);
    _layout->addItem(_spacerItem);
}

void Workspace::removeItem(AbstractWorkspaceItem *item)
{
    _items.removeOne(item);
    _layout->removeWidget(item);
    item->deleteLater();
    updateContentSize();
}

QJsonObject Workspace::serialize() const
{
    QJsonObject json;
    json["name"] = _workspaceName;

    QJsonArray itemArray;
    for (const AbstractWorkspaceItem *item : _items) {
        itemArray.append(item->serialize());
    }
    json["items"] = itemArray;

    return json;
}

void Workspace::deserialize(const QJsonObject &json)
{
    if (json.contains("name")) {
        setName(json["name"].toString());
    }

    if (json.contains("items")) {
        QJsonArray itemArray = json["items"].toArray();
        for (const QJsonValue &itemVal : itemArray) {
            QJsonObject itemObj = itemVal.toObject();
            QString type = itemObj["type"].toString();
            AbstractWorkspaceItem *item = nullptr;

            if (type == "TitleItem") {
                item = new TitleItem();
            } else if (type == "TextItem") {
                item = new TextItem();
            } else if (type == "OrderedListItem") {
                item = new ListItem(ListItem::Ordered);
            } else if (type == "UnorderedListItem") {
                item = new ListItem(ListItem::Unordered);
            } else if (type == "CheckboxItem") {
                item = new CheckboxItem();
            } else if (type == "ImageItem") {
                item = new ImageItem();
            } else if (type == "FileItem") {
                item = new FileItem();
            } else if (type == "NestedWorkspaceItem") {
                item = new NestedWorkspaceItem();
            }

            if (item) {
                item->deserialize(itemObj);
                addItem(item);
            }
        }
    }
}

void Workspace::deserializeItems(const QJsonArray &itemsArray)
{
    for (const QJsonValue &itemVal : itemsArray) {
        QJsonObject itemObj = itemVal.toObject();
        QString type = itemObj["type"].toString();
        AbstractWorkspaceItem *item = nullptr;

        if (type == "TitleItem") {
            item = new TitleItem();
        } else if (type == "TextItem") {
            item = new TextItem();
        } else if (type == "OrderedListItem") {
            item = new ListItem(ListItem::Ordered);
        } else if (type == "UnorderedListItem") {
            item = new ListItem(ListItem::Unordered);
        } else if (type == "CheckboxItem") {
            item = new CheckboxItem();
        } else if (type == "ImageItem") {
            item = new ImageItem();
        } else if (type == "FileItem") {
            item = new FileItem();
        } else if (type == "NestedWorkspaceItem") {
            item = new NestedWorkspaceItem();
        }

        if (item) {
            item->deserialize(itemObj);
            addItem(item);
        }
    }
}

void Workspace::updateContentSize()
{
    int totalHeight = 0;

    for (AbstractWorkspaceItem *item : _items) {
        QRect itemGeometry = item->geometry();
        totalHeight += itemGeometry.height() + _layout->spacing();
    }
    totalHeight += _layout->spacing();

    if (_spacerItem) {
        totalHeight += _spacerItem->geometry().height() + _layout->spacing();
    }

    _contentWidget->setFixedHeight(totalHeight);
    _contentWidget->updateGeometry();
    for (AbstractWorkspaceItem *item : _items) {
        QRect itemGeometry = item->geometry();
    }
}

QList<AbstractWorkspaceItem *> Workspace::getItems() const
{
    return _items;
}

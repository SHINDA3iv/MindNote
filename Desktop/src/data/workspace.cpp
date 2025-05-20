#include "workspace.h"
#include "checkbox_item.h"
#include "file_item.h"
#include "image_item.h"
#include "list_item.h"
#include "text_item.h"
#include "title_item.h"
#include "elements/SubspaceLinkItem.h"

#include <QScrollArea>
#include <QMenu>
#include <QToolButton>
#include <QIcon>
#include <QFileDialog>
#include <QPushButton>
#include <QDebug>
#include <QBuffer>

Workspace::Workspace(const QString &name, QWidget *parent) :
    QWidget(parent),
    _workspaceName(name),
    _iconLabel(new QLabel(this))
{
    this->setStyleSheet(R"(
        QWidget {
            margin:4px;
        }

        Workspace {
            background-color: white;
            border: 2px solid #ddd;
            border-radius: 4px;
            padding: 10px;
        }

        QLabel {
            margin: 5px;
        }

        QToolButton {
            border: none;
            padding: 4px;
            border-radius: 4px;
        }

        QToolButton:hover {
            background-color: #f0f0f0;
        }

        QScrollArea {
            border: 2px solid #ddd;
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
    toolMenu->addSeparator();
    toolMenu->addAction(QIcon::fromTheme("folder"), "Добавить подпространство", this, [this]() {
        emit addSubspaceRequested();
    });

    menuButton->setMenu(toolMenu);
    menuButton->setPopupMode(QToolButton::InstantPopup);

    headerLayout->addWidget(menuButton);

    // --- Кнопки-ссылки на подпространства ---
    if (!_subWorkspaces.isEmpty()) {
        QHBoxLayout *subLinksLayout = new QHBoxLayout();
        for (Workspace *sub : _subWorkspaces) {
            QPushButton *subBtn = new QPushButton(sub->getName(), this);
            subBtn->setFlat(true);
            subBtn->setStyleSheet(
             "color: #0078d7; text-decoration: underline; background: transparent; border: none;");
            connect(subBtn, &QPushButton::clicked, this, [this, sub]() {
                emit subWorkspaceClicked(sub);
            });
            subLinksLayout->addWidget(subBtn);
        }
        headerLayout->addLayout(subLinksLayout);
    }

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

QLabel *Workspace::getIconLabel()
{
    return _iconLabel;
}

QIcon Workspace::getIcon() const
{
    return _icon;
}

void Workspace::setIcon(const QIcon &icon)
{
    if (!icon.isNull()) {
        _icon = icon;
    } else {
        // Устанавливаем иконку по умолчанию, если пользовательская иконка не задана
        _icon = QIcon(":/icons/workspace.png");
    }
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
    } else if (type == "TitleItem") {
        item = new TitleItem("Заголовок", this);
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
    json["id"] = _id;
    json["parentId"] = _parentWorkspace ? _parentWorkspace->getId() : "";

    // Save icon if it exists
    if (!_iconLabel->pixmap()->isNull()) {
        QByteArray iconData;
        QBuffer buffer(&iconData);
        buffer.open(QIODevice::WriteOnly);
        _iconLabel->pixmap()->save(&buffer, "PNG");
        json["icon"] = QString(iconData.toBase64());
    }

    // Save items
    QJsonArray itemArray;
    for (const AbstractWorkspaceItem *item : _items) {
        itemArray.append(item->serialize());
    }
    json["items"] = itemArray;

    qDebug() << "Serializing workspace:" << _workspaceName << "ID:" << _id
             << "Parent:" << (_parentWorkspace ? _parentWorkspace->getId() : "none")
             << "Items:" << _items.size();

    return json;
}

void Workspace::deserialize(const QJsonObject &json)
{
    if (json.contains("name"))
        setName(json["name"].toString());
    if (json.contains("id"))
        setId(json["id"].toString());
    // parentId и subWorkspaces обрабатываются в контроллере

    // Load icon if it exists
    if (json.contains("icon")) {
        QByteArray iconData = QByteArray::fromBase64(json["icon"].toString().toUtf8());
        QPixmap iconPixmap;
        iconPixmap.loadFromData(iconData, "PNG");
        if (!iconPixmap.isNull()) {
            setIcon(QIcon(iconPixmap));
        }
    }

    if (json.contains("items")) {
        QJsonArray itemArray = json["items"].toArray();
        for (const QJsonValue &itemVal : itemArray) {
            QJsonObject itemObj = itemVal.toObject();
            QString type = itemObj["type"].toString();
            AbstractWorkspaceItem *item = nullptr;
            if (type == "TitleItem")
                item = new TitleItem();
            else if (type == "TextItem")
                item = new TextItem();
            else if (type == "OrderedListItem")
                item = new ListItem(ListItem::Ordered);
            else if (type == "UnorderedListItem")
                item = new ListItem(ListItem::Unordered);
            else if (type == "CheckboxItem")
                item = new CheckboxItem();
            else if (type == "ImageItem")
                item = new ImageItem();
            else if (type == "FileItem")
                item = new FileItem();
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

Workspace *Workspace::getParentWorkspace() const
{
    return _parentWorkspace;
}
void Workspace::setParentWorkspace(Workspace *parent)
{
    _parentWorkspace = parent;
}
QList<Workspace *> Workspace::getSubWorkspaces() const
{
    return _subWorkspaces;
}
void Workspace::addSubWorkspace(Workspace *sub)
{
    if (!_subWorkspaces.contains(sub)) {
        _subWorkspaces.append(sub);
        sub->setParentWorkspace(this);
        // Добавляем ссылку-элемент, если его нет
        bool hasLink = false;
        for (auto *item : _items) {
            if (item->type() == "SubspaceLinkItem") {
                auto *link = static_cast<SubspaceLinkItem *>(item);
                if (link->getLinkedWorkspace() == sub) {
                    hasLink = true;
                    break;
                }
            }
        }
        if (!hasLink) {
            auto *linkItem = new SubspaceLinkItem(sub, this);
            connect(linkItem, &SubspaceLinkItem::subspaceLinkClicked, this,
                    &Workspace::subWorkspaceClicked);
            addItem(linkItem);
        }
    }
}
void Workspace::removeSubWorkspace(Workspace *sub)
{
    _subWorkspaces.removeOne(sub);
    if (sub->getParentWorkspace() == this)
        sub->setParentWorkspace(nullptr);
}
bool Workspace::hasSubWorkspaceWithName(const QString &name) const
{
    for (auto *ws : _subWorkspaces)
        if (ws->getName() == name)
            return true;
    return false;
}
QString Workspace::getId() const
{
    return _id;
}
void Workspace::setId(const QString &id)
{
    _id = id;
}
QList<Workspace *> Workspace::getPathChain() const
{
    QList<Workspace *> chain;
    const Workspace *ws = this;
    while (ws) {
        chain.prepend(const_cast<Workspace *>(ws));
        ws = ws->getParentWorkspace();
    }
    return chain;
}
QString Workspace::getFullPathName() const
{
    auto chain = getPathChain();
    QStringList names;
    for (auto *ws : chain) names << ws->getName();
    return names.join("/");
}
QString Workspace::getPath() const
{
    QStringList pathParts;
    const Workspace *current = this;
    while (current) {
        pathParts.prepend(current->getName());
        current = current->getParentWorkspace();
    }
    return pathParts.join("/");
}

Workspace *Workspace::getRootWorkspace()
{
    Workspace *root = this;
    while (root->getParentWorkspace()) {
        root = root->getParentWorkspace();
    }
    return root;
}

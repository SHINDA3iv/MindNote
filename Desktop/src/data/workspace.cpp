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
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QJsonDocument>

Workspace::Workspace(const QString &title, QWidget *parent) :
    QWidget(parent),
    _title(title),
    _status(Status::NotStarted),
    _createdAt(QDateTime::currentDateTime().toString(Qt::ISODate)),
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

    auto headerWidget = new QWidget(this);
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerWidget->setLayout(headerLayout);

    _titleLabel = new QLabel(_title, this);
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
        auto sublinkWidget = new QWidget(this);
        QHBoxLayout *subLinksLayout = new QHBoxLayout(sublinkWidget);
        sublinkWidget->setLayout(subLinksLayout);
        for (Workspace *sub : _subWorkspaces) {
            QPushButton *subBtn = new QPushButton(sub->getTitle(), this);
            subBtn->setFlat(true);
            subBtn->setStyleSheet(
             "color: #0078d7; text-decoration: underline; background: transparent; border: none;");
            connect(subBtn, &QPushButton::clicked, this, [this, sub]() {
                emit subWorkspaceClicked(sub);
            });
            subLinksLayout->addWidget(subBtn);
        }
        headerLayout->addWidget(sublinkWidget);
    }

    contentLayout->addWidget(headerWidget);

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
    // Получаем размеры label
    int width = _iconLabel->width();
    int height = _iconLabel->height();

    // Масштабируем pixmap с сохранением пропорций
    QPixmap pixmap = _icon.pixmap(width, height);
    QPixmap scaledPixmap =
     pixmap.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    _iconLabel->setPixmap(scaledPixmap);
    _iconLabel->setAlignment(Qt::AlignCenter); // Центрируем иконку в label
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

QString Workspace::getTitle() const
{
    return _title;
}

void Workspace::setTitle(const QString &title)
{
    _title = title;
    if (_titleLabel) {
        _titleLabel->setText(title);
    }
}

Workspace::Status Workspace::getStatus() const
{
    return _status;
}

void Workspace::setStatus(Status status)
{
    _status = status;
}

QString Workspace::getStatusString() const
{
    switch (_status) {
    case Status::NotStarted:
        return "not_started";
    case Status::InProgress:
        return "in_progress";
    case Status::Completed:
        return "completed";
    default:
        return "not_started";
    }
}

void Workspace::setStatusFromString(const QString &status)
{
    if (status == "not_started") {
        _status = Status::NotStarted;
    } else if (status == "in_progress") {
        _status = Status::InProgress;
    } else if (status == "completed") {
        _status = Status::Completed;
    }
}

QString Workspace::getCreatedAt() const
{
    return _createdAt;
}

void Workspace::setCreatedAt(const QString &createdAt)
{
    _createdAt = createdAt;
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
    json["title"] = _title;
    json["status"] = getStatusString();
    json["created_at"] = _createdAt;

    // Сериализация элементов
    QJsonArray itemsArray;
    for (const auto &item : _items) {
        itemsArray.append(item->serialize());
    }
    json["elements"] = itemsArray;

    // Сериализация подпространств
    QJsonArray subWorkspacesArray;
    for (const auto &subWorkspace : _subWorkspaces) {
        subWorkspacesArray.append(subWorkspace->serialize());
    }
    json["pages"] = subWorkspacesArray;

    return json;
}

void Workspace::deserialize(const QJsonObject &json)
{
    if (json.contains("title")) {
        setTitle(json["title"].toString());
    }

    if (json.contains("status")) {
        setStatusFromString(json["status"].toString());
    }

    if (json.contains("created_at")) {
        setCreatedAt(json["created_at"].toString());
    }

    // Десериализация элементов
    if (json.contains("elements")) {
        QJsonArray itemsArray = json["elements"].toArray();
        deserializeItems(itemsArray);
    }

    // Десериализация подпространств
    if (json.contains("pages")) {
        QJsonArray pagesArray = json["pages"].toArray();
        for (const QJsonValue &pageValue : pagesArray) {
            if (pageValue.isObject()) {
                Workspace *subWorkspace = new Workspace();
                subWorkspace->deserialize(pageValue.toObject());
                addSubWorkspace(subWorkspace);
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
bool Workspace::hasSubWorkspaceWithTitle(const QString &title) const
{
    for (auto *ws : _subWorkspaces)
        if (ws->getTitle() == title)
            return true;
    return false;
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

QString Workspace::getPath() const
{
    auto chain = getPathChain();
    QStringList names;
    for (auto *ws : chain) names << ws->getTitle();
    return names.join("/");
}

QString Workspace::getVersion() const
{
    return _version;
}

void Workspace::setVersion(const QString &version)
{
    _version = version;
}

Workspace *Workspace::getRootWorkspace()
{
    Workspace *root = this;
    while (root->getParentWorkspace()) {
        root = root->getParentWorkspace();
    }
    return root;
}

QString Workspace::getOwner() const
{
    return _owner;
}

void Workspace::setOwner(const QString &owner)
{
    _owner = owner;
}

// --- ICON SERIALIZATION UTILS ---
QString iconToBase64(const QIcon& icon, int size = 64) {
    if (icon.isNull()) return QString();
    QPixmap pixmap = icon.pixmap(size, size);
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");
    return QString::fromLatin1(ba.toBase64());
}

QIcon base64ToIcon(const QString& base64) {
    QByteArray ba = QByteArray::fromBase64(base64.toLatin1());
    QPixmap pixmap;
    pixmap.loadFromData(ba, "PNG");
    return QIcon(pixmap);
}

// --- BACKEND SERIALIZATION ---
QJsonObject Workspace::serializeBackend(bool isMain) const
{
    QJsonObject json;
    json["title"] = _title;
    json["status"] = (_status == NotStarted   ? "not_started"
                      : _status == InProgress ? "in_progress"
                                              : "completed");
    json["created_at"] = _createdAt;
    json["is_main"] = isMain;

    // --- ICON ---
    if (!_icon.isNull()) {
        json["icon"] = iconToBase64(_icon);
    }

    // Сериализация элементов (items) как elements
    QJsonArray elementsArray;
    for (const AbstractWorkspaceItem *item : _items) {
        elementsArray.append(item->serialize());
    }
    json["elements"] = elementsArray;

    // Сериализация вложенных страниц (subWorkspaces) как pages
    QJsonArray pagesArray;
    for (const Workspace *sub : _subWorkspaces) {
        pagesArray.append(sub->serializeBackend(false));
    }
    json["pages"] = pagesArray;

    return json;
}

void Workspace::deserializeBackend(const QJsonObject &json, bool isMain)
{
    if (json.contains("title"))
        setTitle(json["title"].toString());
    if (json.contains("status")) {
        QString statusStr = json["status"].toString();
        if (statusStr == "not_started")
            _status = NotStarted;
        else if (statusStr == "in_progress")
            _status = InProgress;
        else if (statusStr == "completed")
            _status = Completed;
    }
    if (json.contains("created_at"))
        _createdAt = json["created_at"].toString();

    // --- ICON ---
    if (json.contains("icon")) {
        setIcon(base64ToIcon(json["icon"].toString()));
    }

    // is_main можно использовать для логики, если нужно

    // Десериализация элементов
    if (json.contains("elements")) {
        QJsonArray elementsArray = json["elements"].toArray();
        deserializeItems(elementsArray);
    }

    // Десериализация вложенных страниц
    if (json.contains("pages")) {
        QJsonArray pagesArray = json["pages"].toArray();
        for (const QJsonValue &pageVal : pagesArray) {
            QJsonObject pageObj = pageVal.toObject();
            Workspace *sub = new Workspace();
            sub->deserializeBackend(pageObj, false);
            addSubWorkspace(sub);
        }
    }
}

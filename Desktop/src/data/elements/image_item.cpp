#include "image_item.h"

ImageItem::ImageItem(const QString &imagePath, Workspace *parent) :
    ResizableItem(parent),
    _imageLabel(new QLabel(this)),
    _imagePath(imagePath)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(_imageLabel);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->setSizeConstraint(QLayout::SetFixedSize);

    setLayout(layout);

    if (!imagePath.isEmpty()) {
        QPixmap pixmap(imagePath);
        _originalPixmap = pixmap;

        int initialWidth = qMin(pixmap.width(), 400);
        int initialHeight = (initialWidth * pixmap.height()) / pixmap.width();

        _imageLabel->setFixedSize(initialWidth, initialHeight);
        setFixedSize(initialWidth, initialHeight);

        QPixmap scaledPixmap =
         pixmap.scaled(initialWidth, initialHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        _imageLabel->setPixmap(scaledPixmap);
        _imageLabel->setAlignment(Qt::AlignCenter);
        resize(_imageLabel->pixmap().width(), _imageLabel->pixmap().height());
    } else {
        _imageLabel->setFixedSize(200, 200);
        setFixedSize(200, 200);
    }
}

QString ImageItem::type() const
{
    return "ImageItem";
}

QJsonObject ImageItem::serialize() const
{
    QJsonObject json;
    json["type"] = type();
    json["imagePath"] = _imagePath;
    return json;
}

void ImageItem::deserialize(const QJsonObject &json)
{
    if (json.contains("imagePath")) {
        _imagePath = json["imagePath"].toString();
        QPixmap pixmap(_imagePath);
        _originalPixmap = pixmap;
        updateImageSize();
    }
}

void ImageItem::resizeEvent(QResizeEvent *event)
{
    if (!_imagePath.isEmpty()) {
        resize(event->size());
        _imageLabel->resize(event->size());
        updateImageSize();
    }
    ResizableItem::resizeEvent(event);

    qDebug() << "Позиция" << pos() << _imageLabel->pos();
    qDebug() << "Размер" << size() << _imageLabel->size() << _imageLabel->pixmap().size();
}

void ImageItem::updateImageSize()
{
    if (!_originalPixmap.isNull()) {
        QPixmap scaledPixmap =
         _originalPixmap.scaled(_imageLabel->width(), _imageLabel->height(), Qt::KeepAspectRatio,
                                Qt::SmoothTransformation);
        _imageLabel->setPixmap(scaledPixmap);
        _imageLabel->setAlignment(Qt::AlignCenter);
    }
}

void ImageItem::addCustomContextMenuActions(QMenu *contextMenu)
{
    // Выравнивание по центру
    QAction *alignCenterAction = contextMenu->addAction("Выравнить по центру");
    connect(alignCenterAction, &QAction::triggered, this, [this]() {
        _imageLabel->setAlignment(Qt::AlignCenter);
    });

    // Выравнивание по левому краю
    QAction *alignLeftAction = contextMenu->addAction("Выравнить по левому краю");
    connect(alignLeftAction, &QAction::triggered, this, [this]() {
        _imageLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    });

    // Выравнивание по правому краю
    QAction *alignRightAction = contextMenu->addAction("Выравнить по правому краю");
    connect(alignRightAction, &QAction::triggered, this, [this]() {
        _imageLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    });

    QAction *deleteAction = contextMenu->addAction("Удалить элемент");
    connect(deleteAction, &QAction::triggered, this, &AbstractWorkspaceItem::deleteItem);

    contextMenu->exec(mapToGlobal(pos()));
}

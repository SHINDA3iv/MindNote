#include "image_item.h"
#include <QBuffer>
#include <QDebug>

ImageItem::ImageItem(const QString &imagePath, Workspace *parent) :
    ResizableItem(parent),
    _imageLabel(new QLabel(this))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(_imageLabel);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    if (!imagePath.isEmpty()) {
        QPixmap pixmap(imagePath);
        _originalPixmap = pixmap;
        _imageData = imageToBase64(pixmap);

        // Calculate initial size while respecting minimum size
        int initialWidth = qMax(qMin(pixmap.width(), 500), 250);
        int initialHeight = qMax((initialWidth * pixmap.height()) / pixmap.width(), 250);

        setFixedSize(initialWidth, initialHeight);
        updateImageSize();
    } else {
        setFixedSize(250, 250);
    }

    _imageLabel->installEventFilter(this);
}

QString ImageItem::type() const
{
    return "ImageItem";
}

QString ImageItem::imageToBase64(const QPixmap &pixmap) const
{
    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");
    return QString(imageData.toBase64());
}

QPixmap ImageItem::base64ToImage(const QString &base64String) const
{
    QByteArray imageData = QByteArray::fromBase64(base64String.toUtf8());
    QPixmap pixmap;
    pixmap.loadFromData(imageData, "PNG");
    return pixmap;
}

QJsonObject ImageItem::serialize() const
{
    QJsonObject json;
    json["type"] = type();
    json["imageData"] = _imageData;
    return json;
}

void ImageItem::deserialize(const QJsonObject &json)
{
    if (json.contains("imageData")) {
        _imageData = json["imageData"].toString();
        _originalPixmap = base64ToImage(_imageData);
        updateImageSize();
    }
}

void ImageItem::resizeEvent(QResizeEvent *event)
{
    if (!_imageData.isEmpty()) {
        updateImageSize();
    }
    ResizableItem::resizeEvent(event);
}

void ImageItem::updateImageSize()
{
    if (!_originalPixmap.isNull()) {
        QPixmap scaledPixmap;
        if (_resizeDirection & (Left | Right)) {
            // When resizing horizontally, scale only the width
            scaledPixmap = _originalPixmap.scaled(width(), _originalPixmap.height(),
                                                  Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        } else if (_resizeDirection & (Top | Bottom)) {
            // When resizing vertically, scale only the height
            scaledPixmap = _originalPixmap.scaled(_originalPixmap.width(), height(),
                                                  Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        } else {
            // When resizing diagonally or no specific direction, maintain aspect ratio
            scaledPixmap = _originalPixmap.scaled(width(), height(), Qt::KeepAspectRatio,
                                                  Qt::SmoothTransformation);
        }

        _imageLabel->setFixedSize(width(), height());
        _imageLabel->setPixmap(scaledPixmap);
        _imageLabel->setAlignment(Qt::AlignCenter);
        _imageLabel->setScaledContents(true);
    }
}

void ImageItem::addCustomContextMenuActions(QMenu *contextMenu)
{
    QAction *alignCenterAction = contextMenu->addAction("Выравнить по центру");
    connect(alignCenterAction, &QAction::triggered, this, [this]() {
        _imageLabel->setAlignment(Qt::AlignCenter);
    });

    QAction *alignLeftAction = contextMenu->addAction("Выравнить по левому краю");
    connect(alignLeftAction, &QAction::triggered, this, [this]() {
        _imageLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    });

    QAction *alignRightAction = contextMenu->addAction("Выравнить по правому краю");
    connect(alignRightAction, &QAction::triggered, this, [this]() {
        _imageLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    });
}

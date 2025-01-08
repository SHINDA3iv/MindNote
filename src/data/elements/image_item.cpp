#include "image_item.h"

ImageItem::ImageItem(const QString &imagePath, Workspace *parent) :
    ResizableItem(parent),
    _imageLabel(new QLabel(this)),
    _imagePath(imagePath)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    // _imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // _imageLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(_imageLabel);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    setLayout(layout);

    if (!imagePath.isEmpty()) {
        QPixmap pixmap(imagePath);
        _imageLabel->setPixmap(pixmap.scaled(pixmap.width(), pixmap.height(), Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation));
    }

    resize(_imageLabel->pixmap().width(), _imageLabel->pixmap().height());
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

        _imageLabel->setPixmap(pixmap.scaled(pixmap.width(), pixmap.height(), Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation));
    }
}

void ImageItem::resizeEvent(QResizeEvent *event)
{
    if (!_imagePath.isEmpty()) {
        QPixmap pixmap(_imagePath);
        qDebug() << event->size().width() << event->size().height();
        int maxWidth = event->size().width() - 20;
        int maxHeight = event->size().height() - 20;
        _imageLabel->setPixmap(
         pixmap.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    ResizableItem::resizeEvent(event);
}

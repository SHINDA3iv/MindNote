#include "image_item.h"

ImageItem::ImageItem(const QString &imagePath, Workspace *parent) :
    ResizableItem(parent),
    _imageLabel(new QLabel(this)),
    _imagePath(imagePath)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(_imageLabel);
    setLayout(layout);

    if (!imagePath.isEmpty()) {
        QPixmap pixmap(imagePath);
        _imageLabel->setPixmap(pixmap.scaled(200, 200, Qt::KeepAspectRatio));
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
        _imageLabel->setPixmap(pixmap.scaled(200, 200, Qt::KeepAspectRatio));
    }
}

#include "image_item.h"

ImageItem::ImageItem(const QString &imagePath, QWidget *parent) :
    AbstractWorkspaceItem(parent),
    imageLabel(new QLabel(this)),
    imagePath(imagePath)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(imageLabel);
    setLayout(layout);

    if (!imagePath.isEmpty()) {
        QPixmap pixmap(imagePath);
        imageLabel->setPixmap(pixmap.scaled(200, 200, Qt::KeepAspectRatio));
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
    json["imagePath"] = imagePath;
    return json;
}

void ImageItem::deserialize(const QJsonObject &json)
{
    if (json.contains("imagePath")) {
        imagePath = json["imagePath"].toString();
        QPixmap pixmap(imagePath);
        imageLabel->setPixmap(pixmap.scaled(200, 200, Qt::KeepAspectRatio));
    }
}

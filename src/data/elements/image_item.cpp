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
        int maxWidth = parent ? parent->width() - 20 : 200; // Задаем максимальную ширину
        _imageLabel->setPixmap(
         pixmap.scaled(maxWidth, pixmap.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    resize(width(), height());
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

        int maxWidth =
         parentWidget() ? parentWidget()->width() - 20 : 200; // Задаем максимальную ширину
        _imageLabel->setPixmap(
         pixmap.scaled(maxWidth, pixmap.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void ImageItem::resizeEvent(QResizeEvent *event)
{
    if (!_imagePath.isEmpty()) {
        QPixmap pixmap(_imagePath);
        int maxWidth = event->size().width() - 20; // Задаем максимальную ширину
        _imageLabel->setPixmap(
         pixmap.scaled(maxWidth, pixmap.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    ResizableItem::resizeEvent(event);
}

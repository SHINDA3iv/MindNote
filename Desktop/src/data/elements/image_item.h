#ifndef IMAGEITEM_H
#define IMAGEITEM_H

#include "resizable_item.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QPixmap>
#include <QPointer>

class ImageItem : public ResizableItem
{
    Q_OBJECT

public:
    explicit ImageItem(const QString &imagePath = QString(), Workspace *parent = nullptr);

    QString type() const override;

    QJsonObject serialize() const override;
    void deserialize(const QJsonObject &json) override;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void addCustomContextMenuActions(QMenu *contextMenu) override;

private:
    void updateImageSize();
    QString imageToBase64(const QPixmap &pixmap) const;
    QPixmap base64ToImage(const QString &base64String) const;

    QPointer<QLabel> _imageLabel;
    QString _imagePath;
    QPixmap _originalPixmap;
    QString _imageData;
};

#endif // IMAGEITEM_H

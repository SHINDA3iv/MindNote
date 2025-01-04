#ifndef IMAGEITEM_H
#define IMAGEITEM_H

#include "abstract_workspace_item.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QPixmap>

class ImageItem : public AbstractWorkspaceItem
{
    Q_OBJECT

public:
    explicit ImageItem(const QString &imagePath = "", QWidget *parent = nullptr);

    QString type() const override;

    QJsonObject serialize() const override;
    void deserialize(const QJsonObject &json) override;

private:
    QLabel *imageLabel;
    QString imagePath;
};

#endif // IMAGEITEM_H

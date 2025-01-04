#include "file_item.h"

FileItem::FileItem(const QString &filePath, QWidget *parent) :
    AbstractWorkspaceItem(parent),
    fileButton(new QPushButton(this)),
    filePath(filePath)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(fileButton);
    setLayout(layout);

    if (!filePath.isEmpty()) {
        QFileInfo fileInfo(filePath);
        fileButton->setText(fileInfo.fileName());
    }

    connect(fileButton, &QPushButton::clicked, this, &FileItem::openFile);
}

QString FileItem::type() const
{
    return "FileItem";
}

QJsonObject FileItem::serialize() const
{
    QJsonObject json;
    json["type"] = type();
    json["filePath"] = filePath;
    return json;
}

void FileItem::deserialize(const QJsonObject &json)
{
    if (json.contains("filePath")) {
        filePath = json["filePath"].toString();
        QFileInfo fileInfo(filePath);
        fileButton->setText(fileInfo.fileName());
    }
}

void FileItem::openFile()
{
    // Реализация открытия файла (например, через QDesktopServices::openUrl)
}

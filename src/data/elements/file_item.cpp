#include "file_item.h"

#include <QDesktopServices>

FileItem::FileItem(const QString &filePath, Workspace *parent) :
    AbstractWorkspaceItem(parent),
    _fileButton(new QPushButton(this)),
    _filePath(filePath)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(_fileButton);
    setLayout(layout);

    if (!filePath.isEmpty()) {
        QFileInfo fileInfo(filePath);
        _fileButton->setText(fileInfo.fileName());
    }

    connect(_fileButton, &QPushButton::clicked, this, &FileItem::openFile);
    resize(width(), 50);
}

QString FileItem::type() const
{
    return "FileItem";
}

QJsonObject FileItem::serialize() const
{
    QJsonObject json;
    json["type"] = type();
    json["filePath"] = _filePath;
    return json;
}

void FileItem::deserialize(const QJsonObject &json)
{
    if (json.contains("filePath")) {
        _filePath = json["filePath"].toString();
        QFileInfo fileInfo(_filePath);
        _fileButton->setText(fileInfo.fileName());
    }
}

void FileItem::openFile()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(_filePath));
}

#ifndef FILEITEM_H
#define FILEITEM_H

#include "resizable_item.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QPointer>

class FileItem : public AbstractWorkspaceItem
{
    Q_OBJECT

public:
    explicit FileItem(const QString &filePath = "", Workspace *parent = nullptr);

    QString type() const override;

    QJsonObject serialize() const override;
    void deserialize(const QJsonObject &json) override;

private slots:
    void openFile();

private:
    QPointer<QPushButton> _fileButton;
    QString _filePath;
};

#endif // FILEITEM_H

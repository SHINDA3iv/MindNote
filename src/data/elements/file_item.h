#ifndef FILEITEM_H
#define FILEITEM_H

#include "abstract_workspace_item.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileInfo>

class FileItem : public AbstractWorkspaceItem
{
    Q_OBJECT

public:
    explicit FileItem(const QString &filePath = "", QWidget *parent = nullptr);

    QString type() const override;

    QJsonObject serialize() const override;
    void deserialize(const QJsonObject &json) override;

private slots:
    void openFile();

private:
    QPushButton *fileButton;
    QString filePath;
};

#endif // FILEITEM_H

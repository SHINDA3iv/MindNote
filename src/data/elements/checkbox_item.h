#ifndef CHECKBOXITEM_H
#define CHECKBOXITEM_H

#include "abstract_workspace_item.h"
#include "workspace.h"
#include <qlineedit.h>

#include <QCheckBox>
#include <QVBoxLayout>
#include <QPointer>

class CheckboxItem : public AbstractWorkspaceItem
{
    Q_OBJECT
public:
    explicit CheckboxItem(const QString &label = "Задача", Workspace *parent = nullptr);

    QString type() const override;

    QJsonObject serialize() const override;
    void deserialize(const QJsonObject &json) override;

protected:
    void addCustomContextMenuActions(QMenu *contextMenu) override;

private:
    QPointer<QCheckBox> _checkbox;
    QPointer<QLineEdit> _editLine;

private slots:
    void startEditing();
    void finishEditing();
};

#endif // CHECKBOXITEM_H

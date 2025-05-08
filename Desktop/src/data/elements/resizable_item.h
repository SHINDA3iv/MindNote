#ifndef RESIZABLE_ITEM_H
#define RESIZABLE_ITEM_H

#include "abstract_workspace_item.h"
#include "workspace.h"

#include <QMouseEvent>

class ResizableItem : public AbstractWorkspaceItem
{
    Q_OBJECT
public:
    ResizableItem(Workspace *parent);

    enum ResizeDirection
    {
        None = 0,
        Top = 1,
        Bottom = 2,
        Left = 4,
        Right = 8
    };
    Q_DECLARE_FLAGS(ResizeDirections, ResizeDirection)

signals:
    void resized();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void updateResizeDirection(const QPoint &pos);
    void updateCursor();
    bool eventFilter(QObject *watched, QEvent *event) override;

    bool _resizing;
    ResizeDirections _resizeDirection;
    QPoint _lastMousePos;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ResizableItem::ResizeDirections)

#endif // RESIZABLE_ITEM_H

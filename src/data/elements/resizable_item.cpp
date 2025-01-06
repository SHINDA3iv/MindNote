#include "resizable_item.h"

#include <QCursor>

ResizableItem::ResizableItem(Workspace *parent) :
    AbstractWorkspaceItem(parent),
    _resizing(false),
    _resizeDirection(None)
{}

void ResizableItem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        updateResizeDirection(event->pos());
        if (_resizeDirection != None) {
            _resizing = true;
            _lastMousePos = event->pos();
        }
    }
}

void ResizableItem::mouseMoveEvent(QMouseEvent *event)
{
    if (_resizing) {
        int deltaX = event->pos().x() - _lastMousePos.x();
        int deltaY = event->pos().y() - _lastMousePos.y();

        QRect newGeometry = geometry();

        if (_resizeDirection & Top) {
            newGeometry.setTop(newGeometry.top() + deltaY);
        }
        if (_resizeDirection & Bottom) {
            newGeometry.setBottom(newGeometry.bottom() + deltaY);
        }
        if (_resizeDirection & Left) {
            newGeometry.setLeft(newGeometry.left() + deltaX);
        }
        if (_resizeDirection & Right) {
            newGeometry.setRight(newGeometry.right() + deltaX);
        }

        resize(newGeometry.width(), newGeometry.height());
        _lastMousePos = event->pos();

        emit resized();
    } else {
        updateResizeDirection(event->pos());
        updateCursor();
    }
}

void ResizableItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        _resizing = false;
        _resizeDirection = None;
    }
}

void ResizableItem::updateResizeDirection(const QPoint &pos)
{
    _resizeDirection = None;

    if (pos.y() < 15) {
        _resizeDirection |= Top;
    } else if (pos.y() > height() - 15) {
        _resizeDirection |= Bottom;
    }

    if (pos.x() < 15) {
        _resizeDirection |= Left;
    } else if (pos.x() > width() - 15) {
        _resizeDirection |= Right;
    }
}

void ResizableItem::updateCursor()
{
    switch (_resizeDirection) {
    case Top:
        setCursor(Qt::SizeVerCursor);
        break;
    case Bottom:
        setCursor(Qt::SizeVerCursor);
        break;
    case Left:
        setCursor(Qt::SizeHorCursor);
        break;
    case Right:
        setCursor(Qt::SizeHorCursor);
        break;
    case Top | Left:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case Bottom | Right:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case Top | Right:
        setCursor(Qt::SizeBDiagCursor);
        break;
    case Bottom | Left:
        setCursor(Qt::SizeBDiagCursor);
        break;
    default:
        setCursor(Qt::ArrowCursor);
        break;
    }
}

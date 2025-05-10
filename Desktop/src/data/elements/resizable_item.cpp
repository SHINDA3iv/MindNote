#include "resizable_item.h"

#include <QCursor>

ResizableItem::ResizableItem(Workspace *parent) :
    AbstractWorkspaceItem(parent),
    _resizing(false),
    _resizeDirection(None)
{}

bool ResizableItem::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseMove || 
        event->type() == QEvent::MouseButtonPress || 
        event->type() == QEvent::MouseButtonRelease) {
        
        QWidget *childWidget = qobject_cast<QWidget*>(watched);
        if (childWidget) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            QPoint globalPos = childWidget->mapTo(this, mouseEvent->pos());
            QMouseEvent newEvent(
                event->type(),
                globalPos,
                mouseEvent->button(),
                mouseEvent->buttons(),
                mouseEvent->modifiers()
            );
            
            if (event->type() == QEvent::MouseMove) {
                mouseMoveEvent(&newEvent);
            } else if (event->type() == QEvent::MouseButtonPress) {
                mousePressEvent(&newEvent);
            } else if (event->type() == QEvent::MouseButtonRelease) {
                mouseReleaseEvent(&newEvent);
            }
            return true;
        }
    }
    return QObject::eventFilter(watched, event);
}

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
        bool isShiftPressed = event->modifiers() & Qt::ShiftModifier;

        // Проверяем, не достигнут ли минимальный размер
        if (newGeometry.width() <= 50 && deltaX < 0) deltaX = 0;
        if (newGeometry.height() <= 50 && deltaY < 0) deltaY = 0;

        if (isShiftPressed && (_resizeDirection & (Top | Bottom)) && (_resizeDirection & (Left | Right))) {
            // При нажатом Shift и перетаскивании за угол, масштабируем только по диагонали
            double aspectRatio = static_cast<double>(newGeometry.width()) / newGeometry.height();
            
            // Определяем направление движения мыши
            bool movingRight = deltaX > 0;
            bool movingDown = deltaY > 0;
            
            // Определяем, какое изменение больше по модулю
            if (qAbs(deltaX) > qAbs(deltaY)) {
                // Если изменение по X больше, используем его как основное
                deltaY = static_cast<int>(deltaX / aspectRatio);
            } else {
                // Если изменение по Y больше, используем его как основное
                deltaX = static_cast<int>(deltaY * aspectRatio);
            }
            
            // Проверяем, соответствует ли направление движения диагонали
            bool isValidDiagonal = false;
            
            if (_resizeDirection & (Top | Left)) {
                // Для верхнего левого угла
                isValidDiagonal = (movingRight && !movingDown) || (!movingRight && movingDown);
            } else if (_resizeDirection & (Top | Right)) {
                // Для верхнего правого угла
                isValidDiagonal = (!movingRight && !movingDown) || (movingRight && movingDown);
            } else if (_resizeDirection & (Bottom | Left)) {
                // Для нижнего левого угла
                isValidDiagonal = (!movingRight && !movingDown) || (movingRight && movingDown);
            } else if (_resizeDirection & (Bottom | Right)) {
                // Для нижнего правого угла
                isValidDiagonal = (movingRight && !movingDown) || (!movingRight && movingDown);
            }
            
            // Если движение не по диагонали, игнорируем изменение
            if (!isValidDiagonal) {
                return;
            }
            
            // Проверяем, не достигнем ли минимального размера
            if (newGeometry.width() + deltaX <= 50 || newGeometry.height() + deltaY <= 50) {
                return;
            }
            
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
        } else {
            // Обычное масштабирование без Shift
            if (_resizeDirection & Top) {
                if (newGeometry.height() - deltaY >= 50) {
                    newGeometry.setTop(newGeometry.top() + deltaY);
                }
            }
            if (_resizeDirection & Bottom) {
                if (newGeometry.height() + deltaY >= 50) {
                    newGeometry.setBottom(newGeometry.bottom() + deltaY);
                }
            }
            if (_resizeDirection & Left) {
                if (newGeometry.width() - deltaX >= 50) {
                    newGeometry.setLeft(newGeometry.left() + deltaX);
                }
            }
            if (_resizeDirection & Right) {
                if (newGeometry.width() + deltaX >= 50) {
                    newGeometry.setRight(newGeometry.right() + deltaX);
                }
            }
        }

        setFixedSize(newGeometry.width(), newGeometry.height());
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

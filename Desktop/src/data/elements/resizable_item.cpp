#include "resizable_item.h"

#include <QCursor>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>

ResizableItem::ResizableItem(Workspace *parent) :
    AbstractWorkspaceItem(parent),
    _resizing(false),
    _resizeDirection(None)
{
    // Создаем таймер для обновления размеров
    _resizeTimer = new QTimer(this);
    _resizeTimer->setInterval(16); // ~60 FPS
    connect(_resizeTimer, &QTimer::timeout, this, [this]() {
        if (_resizing) {
            QPoint globalPos = QCursor::pos();
            QPoint localPos = mapFromGlobal(globalPos);
            QMouseEvent event(QEvent::MouseMove, localPos, globalPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            mouseMoveEvent(&event);
        }
    });
}

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
            _resizeTimer->start();
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

        // Вызываем автоскролл при изменении размеров
        autoScrollDuringResize(event->globalPos());

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
        _resizeTimer->stop();
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

void ResizableItem::autoScrollDuringResize(const QPoint &globalPos)
{
    // Находим родительский QScrollArea
    QWidget *parent = this;
    QScrollArea *scrollArea = nullptr;
    
    while (parent) {
        parent = parent->parentWidget();
        if (scrollArea = qobject_cast<QScrollArea*>(parent)) {
            break;
        }
    }
    
    if (!scrollArea) {
        return;
    }

    // Получаем видимую область скролла
    QRect viewportRect = scrollArea->viewport()->rect();
    QPoint viewportPos = scrollArea->viewport()->mapFromGlobal(globalPos);
    
    // Определяем, нужно ли скроллить
    const int scrollMargin = 50; // Отступ от края для начала скролла
    
    if (viewportPos.x() < scrollMargin) {
        // Скролл влево
        scrollArea->horizontalScrollBar()->setValue(
            scrollArea->horizontalScrollBar()->value() - (scrollMargin - viewportPos.x())
        );
    } else if (viewportPos.x() > viewportRect.width() - scrollMargin) {
        // Скролл вправо
        scrollArea->horizontalScrollBar()->setValue(
            scrollArea->horizontalScrollBar()->value() + (viewportPos.x() - (viewportRect.width() - scrollMargin))
        );
    }
    
    if (viewportPos.y() < scrollMargin) {
        // Скролл вверх
        scrollArea->verticalScrollBar()->setValue(
            scrollArea->verticalScrollBar()->value() - (scrollMargin - viewportPos.y())
        );
    } else if (viewportPos.y() > viewportRect.height() - scrollMargin) {
        // Скролл вниз
        scrollArea->verticalScrollBar()->setValue(
            scrollArea->verticalScrollBar()->value() + (viewportPos.y() - (viewportRect.height() - scrollMargin))
        );
    }
}

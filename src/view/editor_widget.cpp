#include "editor_widget.h"

#include <QBoxLayout>

EditorWidget::EditorWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);
}

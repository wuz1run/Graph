#include "MyView.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>

void MyView::wheelEvent(QWheelEvent *event)
{
    QPointF scenePos = event->position();
    if (event->angleDelta().y() > 0) {
        scale(1.2, 1.2);
    } else {
        scale(0.8, 0.8);
    }
    centerOn(scenePos);
}


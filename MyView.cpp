#include "MyView.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>

void MyView::wheelEvent(QWheelEvent *event)
{
    QPointF scenePos = event->position();
    if (event->angleDelta().y() > 0) {
        scale(1.1, 1.1);
        //here too;
    } else {
        scale(0.8, 0.8);
        //this is for git test to show what will happen
    }
    centerOn(scenePos);
}


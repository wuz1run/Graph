#include "MyView.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>
#include "QMessageBox"
#include "widget.h"
void MyView::merge()
{
    if(search)
    {
        QMessageBox::information(nullptr,"搜索模式关闭","搜索模式关闭");
    }
    else
    {
        QMessageBox::information(nullptr,"搜索模式开启","搜索模式开启，请选择起始点（将会以红色标记）");
    }
    search=!search;
}

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
void MyView::mousePressEvent(QMouseEvent *event)
{
    QPoint viewPos = event->pos(); // 视图坐标
    QPointF scenePos = mapToScene(viewPos); // 场景坐标

    if (event->button() == Qt::RightButton&&!search)
    {
        // 右键点击，触发添加点的事件
        emit rightclicked(scenePos.rx(), scenePos.ry());
    }
    else if (event->button() == Qt::LeftButton&&!search)
    {
        // 左键点击，连接点
        emit leftclicked(scenePos.rx(), scenePos.ry());
    }
    else if(event->button() == Qt::LeftButton&&search)
    {
        emit searching(scenePos.rx(), scenePos.ry());
    }

    QGraphicsView::mousePressEvent(event);
}



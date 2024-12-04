#ifndef MYVIEW_H
#define MYVIEW_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QWheelEvent>

class MyView : public QGraphicsView
{
    Q_OBJECT
public:
    MyView(QGraphicsScene *scene) : QGraphicsView(scene) {}

protected:
    void wheelEvent(QWheelEvent *event) override;         //鼠标滚轮事件
    void mousePressEvent(QMouseEvent *event) override;
private:
    bool isDragging = false;  // 标记是否正在拖动
    QPointF lastScenePos;     // 记录上一次鼠标的位置
    bool capture;
signals:
    void rightclicked(qreal x,qreal y);
    void leftclicked(qreal x,qreal y);


};
#endif // MYVIEW_H

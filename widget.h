#ifndef WIDGET_H
#define WIDGET_H
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QWidget>
#include "QGraphicsView"
#include "QVBoxLayout"
#include "QFile"
QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    void onMouseClicked(const QPointF &scenePos);
    void onButtonClicked();
    void savePoint(qreal x,qreal y,QString name);
private:
    Ui::Widget *ui;
    QVBoxLayout *layout;
    QGraphicsScene *scene;
    bool captureMouseClick;
    QJsonArray pointsArray;
    QJsonArray linesArray;
    QList<QGraphicsEllipseItem *> points;  // 存储所有的点
    QList<QGraphicsLineItem *> lines;  // 存储所有的线
    void savePointsToJson();
    void loadPointsFromJson();
    void loadLinesFromJson();
    void loadPoint(qreal x,qreal y);
    void loadLine(qreal x1,qreal y1,qreal x2,qreal y2);
    void clickToAddPoint(qreal x,qreal y);
    void clickToConnectPoints(qreal x,qreal y);
    void saveLinesToJson();
    QGraphicsLineItem *currentLine = nullptr;  // 当前的连接线
    QGraphicsEllipseItem *firstSelectedPoint = nullptr;  // 第一个被选中的点
};
#endif // WIDGET_H
class MyGraphicsView :QGraphicsView
{
    Q_OBJECT

public:
    MyGraphicsView(QGraphicsScene *scene) : QGraphicsView(scene) {}



protected:

private:


};

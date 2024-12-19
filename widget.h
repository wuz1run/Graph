#ifndef WIDGET_H
#define WIDGET_H
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QWidget>
#include "QGraphicsView"
#include "QVBoxLayout"
#include "QFile"
#include "QLabel"
#include "QTimer"
QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
private:
    Ui::Widget *ui;
    QVBoxLayout *layout;
    QGraphicsScene *scene;
    bool captureMouseClick;
    QJsonArray pointsArray;
    QJsonArray linesArray;
    QList<QGraphicsEllipseItem *> points;  // 存储所有的点
    QList<QGraphicsLineItem *> lines;  // 存储所有的线
    QList<QGraphicsLineItem *> redlines;
    QGraphicsLineItem *currentLine = nullptr;  // 当前的连接线
    QGraphicsEllipseItem *firstSelectedPoint = nullptr;
    QGraphicsEllipseItem *firstSearchingPoint = nullptr;
    QLabel *label;
    QLabel* distanceLabel;
    bool status=false;
    QTimer *timer;
    void savePointsToJson();
    void loadPointsFromJson();
    void loadLinesFromJson();
    void loadPoint(qreal x,qreal y,const QString &name);
    void loadLine(qreal x1,qreal y1,qreal x2,qreal y2);
    void clickToAddPoint(qreal x,qreal y);
    void clickToConnectPoints(qreal x,qreal y);
    void onMouseClicked(const QPointF &scenePos);
    void onButtonClicked();
    void savePoint(qreal x,qreal y,QString name);
    double calculateDistance(const QPointF &p1, const QPointF &p2);
    bool readJsonFromFile(const QString &fileName, QList<QPointF> &points, QList<QLineF> &lines);
    QList<int> dijkstra(const QList<QPointF> &points, const QList<QLineF> &lines, int startIndex, int endIndex);
    QList<QPointF> getPathPoints(const QList<QPointF> &points, const QList<int> &pathIndices);
    void getTwoPoints(qreal x,qreal y);
    void findWay(QPair<QPointF,QPointF> pair);
    void findWayByName();
    void loadColoredLine(qreal x1, qreal y1, qreal x2, qreal y2);
    void removeRedLines(QGraphicsScene *scene);
    QMap<QGraphicsEllipseItem*, QString> pointNames;
    QGraphicsTextItem* getTextForItem(QGraphicsItem *item);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) ;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void deletePoint();
    void reverseStatus(bool a);
    void timeout();


};
#endif // WIDGET_H


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
    void savePointsToJson();
    void loadPointsFromJson();
    void loadPoint(qreal x,qreal y);
    void loadLine(QString pos1,QString pos2);
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

#include "widget.h"
#include "ui_widget.h"
#include "MyView.h"
#include "QVBoxLayout"
#include <QGraphicsRectItem>
#include "QPushButton"
#include "QMessageBox"
#include "QInputDialog"
#include "location.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget), scene(new QGraphicsScene(this))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    this->setLayout(layout);

    // 创建矩形项

    scene->setSceneRect(-100, -100, 800, 600);
    QPixmap background("C:/Users/wuzir/Documents/Graph/map.jpg");  // 你的背景图片路径

    // 创建一个 QGraphicsPixmapItem 并将其添加到场景中
    QGraphicsPixmapItem *backgroundItem = new QGraphicsPixmapItem(background);
    backgroundItem->setTransformationMode(Qt::SmoothTransformation);
    backgroundItem->setZValue(-1);  // 设置 Z 值，让背景位于最底层
    backgroundItem->setPixmap(background.scaled(scene->sceneRect().size().toSize()));
    scene->addItem(backgroundItem);

    // 创建自定义视图 MyView
    MyView *MV = new MyView(scene);

    layout->addWidget(MV);  // 将视图添加到布局中

    // 创建按钮
    QPushButton *button1 = new QPushButton("Clear all");
    layout->addWidget(button1);
    // 设置视图的拖动模式
    MV->setDragMode(QGraphicsView::ScrollHandDrag);
    connect(button1, &QPushButton::clicked, this, &Widget::onButtonClicked);
    loadPointsFromJson();
    connect(MV, &MyView::rightclicked, this, &Widget::clickToAddPoint);
    connect(MV, &MyView::leftclicked, this, &Widget::clickToConnectPoints);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::onMouseClicked(const QPointF &scenePos)
{
    qDebug() << "Mouse clicked at position in Widget:" << scenePos;

    // 弹出对话框显示坐标
    QMessageBox::information(nullptr, "Mouse Position",
                             QString("Mouse clicked at:\nX: %1, Y: %2")
                                 .arg(scenePos.x()).arg(scenePos.y()));
}

void Widget::onButtonClicked()
{
    for (QGraphicsEllipseItem *point : points)
    {
        scene->removeItem(point);
        delete point;
    }
    for (QGraphicsLineItem *line : lines)
    {
        scene->removeItem(line);
        delete line;
    }

    // 清空点和线的列表
    points.clear();
    lines.clear();

    // 清空 JSON 数据文件
    QFile file("C:\\Users\\wuzir\\Documents\\Graph\\points_and_lines.json");
    if (file.exists())
    {
        file.remove();
    }
}

void Widget::savePoint(qreal x, qreal y, QString name)
{
    qDebug() << "saving" << Qt::endl;
    QJsonObject pointObject;
    pointObject["x"] = x;
    pointObject["y"] = y;
    pointObject["name"] = name;

    // 将点的 JSON 对象添加到 JSON 数组中
    pointsArray.append(pointObject);

    // 将 JSON 数组保存到文件
    savePointsToJson();
}

// 鼠标点击事件
void MyView::mousePressEvent(QMouseEvent *event)
{
    QPoint viewPos = event->pos(); // 视图坐标
    QPointF scenePos = mapToScene(viewPos); // 场景坐标

    if (event->button() == Qt::RightButton)
    {
        // 右键点击，触发添加点的事件
        emit rightclicked(scenePos.rx(), scenePos.ry());
    }
    else if (event->button() == Qt::LeftButton)
    {
        // 左键点击，连接点
        emit leftclicked(scenePos.rx(), scenePos.ry());
    }

    QGraphicsView::mousePressEvent(event);
}

void Widget::savePointsToJson()
{
    // 创建一个 JSON 文档，首先保存点的信息
    QJsonObject docObject;

    // 保存点的数据
    QJsonArray pointsJsonArray;
    for (QGraphicsEllipseItem *point : points)
    {
        QJsonObject pointObject;
        pointObject["x"] = point->pos().x();
        pointObject["y"] = point->pos().y();
        pointsJsonArray.append(pointObject);
    }

    docObject["points"] = pointsJsonArray;

    // 保存线的数据
    QJsonArray linesJsonArray = linesArray;  // 直接使用保存的线数据
    docObject["lines"] = linesJsonArray;

    // 将 JSON 文档写入文件
    QFile file("C:\\Users\\wuzir\\Documents\\Graph\\points_and_lines.json");
    if (file.open(QIODevice::WriteOnly))
    {
        QJsonDocument doc(docObject);
        file.write(doc.toJson());
        file.close();
    }
    else
    {
        qDebug() << "Failed to open points_and_lines.json for writing.";
    }
}

void Widget::loadPointsFromJson()
{
    QFile file("C:\\Users\\wuzir\\Documents\\Graph\\points_and_lines.json");
    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray data = file.readAll();
        file.close();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject())
        {
            QJsonObject docObject = doc.object();

            // 加载点的数据
            QJsonArray pointsJsonArray = docObject["points"].toArray();
            for (const QJsonValue &value : pointsJsonArray)
            {
                QJsonObject pointObject = value.toObject();
                qreal x = pointObject["x"].toDouble();
                qreal y = pointObject["y"].toDouble();
                loadPoint(x, y);
            }

            // 加载线的数据
            QJsonArray linesJsonArray = docObject["lines"].toArray();
            for (const QJsonValue &value : linesJsonArray)
            {
                QJsonObject lineObject = value.toObject();
                qreal x1 = lineObject["x1"].toDouble();
                qreal y1 = lineObject["y1"].toDouble();
                qreal x2 = lineObject["x2"].toDouble();
                qreal y2 = lineObject["y2"].toDouble();

                // 重新绘制线
                QGraphicsLineItem *lineItem = new QGraphicsLineItem();
                lineItem->setLine(QLineF(x1, y1, x2, y2));
                lineItem->setPen(QPen(Qt::blue, 2));  // 设置线条颜色和宽度
                scene->addItem(lineItem);
            }
        }
    }
    else
    {
        qDebug() << "Failed to open points_and_lines.json for reading.";
    }
}

void Widget::loadPoint(qreal x, qreal y)
{
    // 创建半径较小的圆（例如，半径为 10）
    QGraphicsEllipseItem *circleItem = new QGraphicsEllipseItem();
    circleItem->setRect(-10, -10, 20, 20);  // 圆的大小是 20x20，中心点在 (x, y)
    circleItem->setPen(QPen(Qt::black));
    circleItem->setBrush(QBrush(Qt::yellow));
    scene->addItem(circleItem);

    // 设置点的位置
    circleItem->setPos(x, y);  // 设置点的实际位置

    points.append(circleItem);  // 将点添加到列表中
}

void Widget::clickToAddPoint(qreal x, qreal y)
{
    loadPoint(x, y);
    QString name = QInputDialog::getText(nullptr, "Enter name", "Enter name");
    savePoint(x, y, name);
}

void Widget::loadLine(qreal x1, qreal y1, qreal x2, qreal y2)
{
    // 创建一条线
    QGraphicsLineItem *lineItem = new QGraphicsLineItem();
    lineItem->setLine(QLineF(x1, y1, x2, y2));
    lineItem->setPen(QPen(Qt::blue, 2));  // 设置线条颜色和宽度
    scene->addItem(lineItem);

    linesArray.append(QJsonObject{
        {"x1", x1},
        {"y1", y1},
        {"x2", x2},
        {"y2", y2}
    });

    savePointsToJson();
}

void Widget::clickToConnectPoints(qreal x, qreal y)
{
    const qreal tolerance = 30;  // 将容忍范围减小为 10 像素

    for (QGraphicsEllipseItem *point : points)
    {
        // 获取点的位置
        QPointF pointPos = point->pos();
        qDebug() << "Point position: " << pointPos;

        // 判断鼠标点击的位置是否接近点
        qreal distance = QLineF(QPointF(x, y), pointPos).length();
        if (distance <= tolerance)  // 如果距离小于容忍范围，认为点击到该点
        {
            if (firstSelectedPoint == nullptr)
            {
                // 选择第一个点
                firstSelectedPoint = point;
                qDebug() << "First point selected at:" << pointPos;
            }
            else
            {
                // 选择第二个点并画线
                QGraphicsLineItem *lineItem = new QGraphicsLineItem();
                lineItem->setLine(QLineF(firstSelectedPoint->pos(), point->pos()));
                lineItem->setPen(QPen(Qt::blue, 2));  // 设置线条颜色和宽度
                scene->addItem(lineItem);
                qDebug() << "Connecting points with line";

                // 将这条线的信息保存到 linesArray 中
                QJsonObject lineObject;
                lineObject["x1"] = firstSelectedPoint->pos().x();
                lineObject["y1"] = firstSelectedPoint->pos().y();
                lineObject["x2"] = point->pos().x();
                lineObject["y2"] = point->pos().y();
                linesArray.append(lineObject);  // 保存线的 JSON 数据
                lines.append(lineItem);
                savePointsToJson();
                // 连接完后清空选择的点
                firstSelectedPoint = nullptr;
            }
            break;
        }
    }
}

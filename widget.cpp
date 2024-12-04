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
    QGraphicsRectItem *rectItem = new QGraphicsRectItem();
    rectItem->setRect(0, 0, 100, 100);
    scene->setSceneRect(-100, -100, 800, 600);
    QPixmap background("C:/Users/wuzir/Documents/Graph/map.jpg");  // 你的背景图片路径

    // 创建一个 QGraphicsPixmapItem 并将其添加到场景中
    QGraphicsPixmapItem *backgroundItem = new QGraphicsPixmapItem(background);
    backgroundItem->setTransformationMode(Qt::SmoothTransformation);
    backgroundItem->setZValue(-1);  // 设置 Z 值，让背景位于最底层
    backgroundItem->setPixmap(background.scaled(scene->sceneRect().size().toSize()));
    scene->addItem(backgroundItem);
    // 将矩形项添加到场景
    scene->addItem(rectItem);

    // 创建自定义视图 MyView
    MyView *MV = new MyView(scene);

    layout->addWidget(MV);  // 将视图添加到布局中

    // 创建按钮
    QPushButton *button1 = new QPushButton("Click Me");
    QPushButton *button2 = new QPushButton("Clear Points");
    layout->addWidget(button1);
    layout->addWidget(button2);
    // 设置视图的拖动模式
    MV->setDragMode(QGraphicsView::ScrollHandDrag);
    connect(button1, &QPushButton::clicked, this, &Widget::onButtonClicked);
    loadPointsFromJson();

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
    bool ok;
    qreal x_pos = QInputDialog::getDouble(nullptr, "Input x pos", "Enter x position:", 0, -1000, 1000, 2, &ok);
    if (!ok) return; // 如果用户点击取消或输入无效则返回

    qreal y_pos = QInputDialog::getDouble(nullptr, "Input y pos", "Enter y position:", 0, -1000, 1000, 2, &ok);
    if (!ok) return; // 如果用户点击取消或输入无效则返回
    QString name = QInputDialog::getText(nullptr,"Input point name","Enter point name");
    // 创建一个新的矩形项，并设置其位置
    loadPoint(x_pos,y_pos);
    savePoint(x_pos,y_pos,name);
}

void Widget::savePoint(qreal x,qreal y,QString name)
{
    qDebug() << "saving"<<Qt::endl;
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
    // 获取鼠标点击的位置（视图坐标系中的位置）
    QPoint viewPos = event->pos(); // 转换为QPoint类型，适应mapToScene的需求

    // 将视图坐标转换为场景坐标
    QPointF scenePos = mapToScene(viewPos);

    // 打印鼠标点击的坐标（可以选择其他方式显示）
    qDebug() << "Mouse clicked at scene position:" << scenePos;

    QGraphicsView::mousePressEvent(event);
    // 可选择弹出对话框显示坐标
}
void Widget::savePointsToJson()
{
    // 创建一个 JSON 文档，将点的数组写入文件
    QJsonDocument doc(pointsArray);
    QFile file("C:\\Users\\wuzir\\Documents\\Graph\\points.json");
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(doc.toJson());
        file.close();
    }
    else
    {
        qDebug() << "Failed to open points.json for writing.";
    }
}
void Widget::loadPointsFromJson()
{
    QFile file("C:\\Users\\wuzir\\Documents\\Graph\\points.json");
    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray data = file.readAll();
        file.close();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isArray())
        {
            pointsArray = doc.array();
            // 遍历所有点
            for (const QJsonValue &value : pointsArray)
            {
                QJsonObject pointObject = value.toObject();
                qreal x = pointObject["x"].toDouble();
                qreal y = pointObject["y"].toDouble();
                QString name = pointObject["name"].toString();
                loadPoint(x,y);
                qDebug() << "Loaded point:" << name << "at" << x << y;
            }
        }
    }
    else
    {
        qDebug() << "Failed to open points.json for reading.";
    }
}
void Widget::loadPoint(qreal x, qreal y)
{
     QGraphicsEllipseItem *circleItem = new QGraphicsEllipseItem();
    circleItem->setRect(x-12.5, y-12.5, 25, 25);
    circleItem->setPen(QPen(Qt::black));
    circleItem->setBrush(QBrush(Qt::yellow));
    scene->addItem(circleItem);
}
void Widget::loadLine(QString pos1, QString pos2)
{

}




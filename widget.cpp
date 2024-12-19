#include "widget.h"
#include "ui_widget.h"
#include "MyView.h"
#include "QVBoxLayout"
#include <QGraphicsRectItem>
#include "QPushButton"
#include "QMessageBox"
#include "QInputDialog"
#include "QRandomGenerator"
#include <QGraphicsSceneHoverEvent>
#include <QTimer>
#include "QThread"
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget), scene(new QGraphicsScene(this))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    this->setLayout(layout);
    scene->setSceneRect(-100, -100, 800, 600);
    QPixmap background(":/map.jpg");
    QGraphicsPixmapItem *backgroundItem = new QGraphicsPixmapItem(background);
    backgroundItem->setTransformationMode(Qt::SmoothTransformation);
    backgroundItem->setZValue(-1);
    backgroundItem->setPixmap(background.scaled(scene->sceneRect().size().toSize()));
    scene->addItem(backgroundItem);
    MyView *MV = new MyView(scene);
    layout->addWidget(MV);
    label=new QLabel(this);
    label->setText("按左键以添加点与点之间的连线，按右键以添加点");
    distanceLabel = new QLabel(this);
    layout->addWidget(label);
    QPushButton *button1 = new QPushButton("Clear all");
    QPushButton *button2= new QPushButton("Find Way");
    QPushButton *button3 = new QPushButton("Delete Point/Line");
    QPushButton *button4 = new QPushButton("Find Way By Name");
    timer= new QTimer();
    layout->addWidget(button1);
    layout->addWidget(button2);
    layout->addWidget(button3);
    layout->addWidget(button4);
    MV->setDragMode(QGraphicsView::ScrollHandDrag);
    connect(button1, &QPushButton::clicked, this, &Widget::onButtonClicked);
    connect(button2, &QPushButton::clicked,MV, &MyView::merge);
    connect(button2, &QPushButton::clicked,this, &Widget::reverseStatus);
    connect(button3,&QPushButton::clicked,this,&Widget::deletePoint);
    connect(button4,&QPushButton::clicked,this,&Widget::findWayByName);
    loadPointsFromJson();
    connect(MV, &MyView::rightclicked, this, &Widget::clickToAddPoint);
    connect(MV, &MyView::leftclicked, this, &Widget::clickToConnectPoints);
    connect(MV,&MyView::searching,this,&Widget::getTwoPoints);
    connect(timer,&QTimer::timeout,this,&Widget::timeout);
}

Widget::~Widget()
{
    savePointsToJson();
    delete ui;

}
/*页面第一个按钮，点击之后删除所有的点*/
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
    points.clear();
    lines.clear();
    QFile file("C:/Users/wuzir/Documents/GitHub/Graph/points_and_lines.json");
    if (file.exists())
    {
        file.remove();
    }
}

/*计算欧几里得距离*/
double Widget::calculateDistance(const QPointF &p1, const QPointF &p2)
{
    return std::sqrt(std::pow(p2.x() - p1.x(), 2) + std::pow(p2.y() - p1.y(), 2));
}
/*把Json中的点和线转化为线边集*/
bool Widget::readJsonFromFile(const QString &fileName, QList<QPointF> &points, QList<QLineF> &lines) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to open file for reading";
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qWarning() << "Failed to parse JSON data";
        return false;
    }
    QJsonObject obj = doc.object();
    QJsonArray pointsArray = obj["points"].toArray();
    for (const QJsonValue &value : pointsArray) {
        QJsonObject pointObj = value.toObject();
        points.append(QPointF(pointObj["x"].toDouble(), pointObj["y"].toDouble()));
    }
    QJsonArray linesArray = obj["lines"].toArray();
    for (const QJsonValue &value : linesArray) {
        QJsonObject lineObj = value.toObject();
        lines.append(QLineF(QPointF(lineObj["x1"].toDouble(), lineObj["y1"].toDouble()),
                            QPointF(lineObj["x2"].toDouble(), lineObj["y2"].toDouble())));
    }

    return true;
}
/*Dijkstra算法核心*/
QList<int> Widget::dijkstra(const QList<QPointF> &points, const QList<QLineF> &lines, int startIndex, int endIndex) {
    int n = points.size();
    QVector<double> dist(n, std::numeric_limits<double>::infinity());
    QVector<int> prev(n, -1);
    QSet<int> unvisited;

    // 初始化
    dist[startIndex] = 0;
    for (int i = 0; i < n; ++i) {
        unvisited.insert(i);
    }

    while (!unvisited.isEmpty()) {
        // 找到距离最小的节点
        int u = -1;
        double minDist = std::numeric_limits<double>::infinity();
        for (int v : unvisited) {
            if (dist[v] < minDist) {
                minDist = dist[v];
                u = v;
            }
        }
        // 如果目标点已找到，退出
        if (u == endIndex) break;

        // 从集合中移除u
        unvisited.remove(u);

        // 更新邻接点的距离
        for (const QLineF &line : lines) {
            int v = -1;
            if (line.p1() == points[u]) {
                v = points.indexOf(line.p2());
            } else if (line.p2() == points[u]) {
                v = points.indexOf(line.p1());
            }

            if (v != -1 && unvisited.contains(v)) {
                double alt = dist[u] + calculateDistance(points[u], points[v]);
                if (alt < dist[v]) {
                    dist[v] = alt;
                    prev[v] = u;
                }
            }
        }
    }
    QList<int> path;
    for (int u = endIndex; u != -1; u = prev[u]) {
        path.prepend(u);
    }

    return path;
}
/*寻路绘制*/
void Widget::findWay(QPair<QPointF, QPointF> pair)
{
    QList<QPointF> points;
    QList<QLineF> lines;

    if (readJsonFromFile("C:/Users/wuzir/Documents/GitHub/Graph/points_and_lines.json", points, lines)) {
        qDebug() << points;
        qDebug() << lines;
    }

    int startIndex = 1;
    int endIndex = 0;
    for (int i = 0; i < points.size(); i++) {
        if (pair.first.rx() == points[i].rx() && pair.first.ry() == points[i].ry()) {
            startIndex = i;
        }
    }
    for (int i = 0; i < points.size(); i++) {
        if (pair.second.rx() == points[i].rx() && pair.second.ry() == points[i].ry()) {
            endIndex = i;
        }
    }

    // 计算最短路径的索引
    QList<int> pathIndices = dijkstra(points, lines, startIndex, endIndex);
    QList<QPointF> pathPoints = getPathPoints(points, pathIndices);
    qDebug() << "Shortest path:" << pathPoints;

    // 计算路径总距离
    double totalDistance = 0;
    QPointF firstPoint = pathPoints[0];

    // 绘制路径并累加总距离
    for (int i = 1; i < pathPoints.size(); i++) {
        // 计算当前两个点之间的距离
        QLineF line(firstPoint, pathPoints[i]);
        totalDistance += line.length();  // 累加每段线的长度

        // 绘制路径
        loadColoredLine(firstPoint.rx(), firstPoint.ry(), pathPoints[i].rx(), pathPoints[i].ry());
           qDebug()<<"sleep";


        update();

        firstPoint = pathPoints[i];  // 更新起点
    }

    // 输出总距离
    qDebug() << "Total distance: " << totalDistance;

    // 显示总距离

    distanceLabel->setText("Distance: " + QString::number(totalDistance*2.5, 'f', 3) + " units");
}


    void Widget::findWayByName()
    {
        // 弹框输入起点和终点的名称

        bool ok;
        qDebug()<<pointNames;
        QString startName = QInputDialog::getText(nullptr, tr("Input Start Point"), tr("Enter start point name:"), QLineEdit::Normal, "", &ok);
        if (!ok || startName.isEmpty()) return;

        QString endName = QInputDialog::getText(nullptr, tr("Input End Point"), tr("Enter end point name:"), QLineEdit::Normal, "", &ok);
        if (!ok || endName.isEmpty()) return;

        // 根据名称查找对应的点
        QGraphicsEllipseItem* startPoint = nullptr;
        QGraphicsEllipseItem* endPoint = nullptr;

        for (auto it = pointNames.begin(); it != pointNames.end(); ++it) {
            if (it.value() == startName) {
                startPoint = it.key();
            }
            if (it.value() == endName) {
                endPoint = it.key();
            }
        }

        if (!startPoint || !endPoint) {
            QMessageBox::warning(this, tr("Error"), tr("Invalid start or end point name"));
            return;
        }

        // 获取点的坐标
        QPointF startPos = startPoint->pos();
        QPointF endPos = endPoint->pos();

        // 调用已有的 findWay 函数，传递点的坐标
        findWay(QPair<QPointF, QPointF>(startPos, endPos));
        QTimer::singleShot(5000, this, [&]() {
           removeRedLines(scene);
        });

    }


/*点集转化为int类型的点集*/
QList<QPointF> Widget::getPathPoints(const QList<QPointF> &points, const QList<int> &pathIndices) {
    QList<QPointF> pathPoints;
    for (int index : pathIndices) {
        pathPoints.append(points[index]);
    }
    return pathPoints;
}
/*移除前一条的路*/
void Widget::removeRedLines(QGraphicsScene *scene) {
    for (int i = redlines.size() - 1; i >= 0; --i) {
        QGraphicsLineItem* line = redlines[i];
        if (line->pen().color() == Qt::red) {
            scene->removeItem(line);
            delete line;  // 删除对象，防止内存泄漏
            redlines.removeAt(i);  // 从容器中移除
        }
    }

    for (QGraphicsEllipseItem *point : points) {
        if (point->brush().color() == Qt::red || point->brush().color() == Qt::blue) {
            point->setBrush(Qt::yellow);  // 修改颜色为黄色
        }
    }

}

void Widget::deletePoint()
{
    QList<QGraphicsEllipseItem *> selectedPoints;
    QList<QGraphicsLineItem *> selectedLines;
    QList<QGraphicsLineItem *> linesToDelete;
    for (QGraphicsEllipseItem *point : points)
    {
        if (point->isSelected())  // 判断点是否被选中
        {
            selectedPoints.append(point);
        }
    }
    for (QGraphicsLineItem *line : lines)
    {
        if (line->isSelected())
        {
            linesToDelete.append(line);
            selectedLines.append(line);
        }
    }
    // 删除与选中的点连接的线
    for (QGraphicsEllipseItem *point : selectedPoints)
    {
        // 遍历所有线，检查是否连接到该点

        for (QGraphicsLineItem *line : lines)
        {
            // 如果线的起点或终点是当前被删除的点
            if (line->line().p1() == point->pos() || line->line().p2() == point->pos())
            {
                linesToDelete.append(line);
            }
        }

        // 删除与点连接的线

    }
    for (QGraphicsLineItem *line : linesToDelete)
    {
        scene->removeItem(line);
        delete line;
        lines.removeAll(line);  // 从线列表中移除
    }

    if( firstSelectedPoint)
    {
        firstSelectedPoint=nullptr;
    }
    for (QGraphicsEllipseItem *point : selectedPoints)
    {
        scene->removeItem(point);
        delete point;
        points.removeAll(point);
    }
    savePointsToJson();
    // 如果没有选中的点或线，提示用户
    if (selectedPoints.isEmpty()&&selectedLines.isEmpty()) {
        QMessageBox::information(this, "Info", "No point selected for deletion.");
    }


}

void Widget::reverseStatus(bool a)
{
    qDebug()<<a;
    removeRedLines(scene);
    status=!status;
}

void Widget::timeout()
{

}



/*得到寻路的两个点*/
void Widget::getTwoPoints(qreal x,qreal y)
{
    if(!firstSearchingPoint)
    {
        removeRedLines(scene);
    }
    const qreal tolerance = 20;
    for (QGraphicsEllipseItem *point : points)
    {
        QPointF pointPos = point->pos();
        qreal distance = QLineF(QPointF(x, y), pointPos).length();
        if (distance <= tolerance)  // 如果距离小于容忍范围，认为点击到该点
        {
            if (firstSearchingPoint == nullptr)
            {
                firstSearchingPoint = point;
                point->setBrush(Qt::red);
                qDebug() << "First point selected at:" << pointPos;
                label->setText("选择第二个点");
            }
            else
            {
                if(firstSearchingPoint->pos()==point->pos())
                {
                    QMessageBox::warning(nullptr,"Error","请重新选择");
                }
                QPointF tempx=firstSearchingPoint->scenePos();
                QPointF tempy=point->scenePos();
                point->setBrush(QBrush(Qt::blue));
                findWay({tempx,tempy});
                if(firstSearchingPoint)
                {
                    firstSearchingPoint = nullptr;
                }
                break;
            }
        }
    }

}
/*把点保存到Json里*/
void Widget::savePoint(qreal x, qreal y, QString name)
{
    QJsonObject pointObject;
    pointObject["x"] = x;
    pointObject["y"] = y;
    pointObject["name"] = name;  // 保存点的名字
    pointsArray.append(pointObject);  // 将点的 JSON 对象添加到 pointsArray
    savePointsToJson();  // 保存所有点和线
}
/*加载一个点*/
void Widget::loadPoint(qreal x, qreal y, const QString &name)
{
    // 创建圆形项
    QGraphicsEllipseItem *circleItem = new QGraphicsEllipseItem();
    circleItem->setRect(-12.5, -12.5, 25, 25);  // 设置矩形区域，使圆心对齐
    circleItem->setPen(QPen(Qt::black));
    circleItem->setBrush(QBrush(Qt::yellow));
    circleItem->setPos(x, y);  // 设置圆形的位置
    circleItem->setData(0,name);
    circleItem->setFlag(QGraphicsItem::ItemIsSelectable);
    pointNames.insert(circleItem, name);
    // 创建文本项并设置位置为圆心
    QGraphicsTextItem *textItem = new QGraphicsTextItem(name, circleItem);
    textItem->setPos(-textItem->boundingRect().width() / 2, -textItem->boundingRect().height() / 2);  // 将文本定位到圆心

    // 添加到场景中
    scene->addItem(circleItem);
    scene->addItem(textItem);

    // 将点添加到点的列表中
    points.append(circleItem);
}


/*加载一条线*/
void Widget::loadLine(qreal x1, qreal y1, qreal x2, qreal y2)
{
    // 创建一条线
    QGraphicsLineItem *lineItem = new QGraphicsLineItem();
    lineItem->setLine(QLineF(x1, y1, x2, y2));
    lineItem->setPen(QPen(Qt::blue, 2));  // 设置线条颜色和宽度
    scene->addItem(lineItem);
    lineItem->setFlag(QGraphicsItem::ItemIsSelectable);
    linesArray.append(QJsonObject{
        {"x1", x1},
        {"y1", y1},
        {"x2", x2},
        {"y2", y2}
    });
    lines.append(lineItem);
}
/*带颜色的线*/
void Widget::loadColoredLine(qreal x1, qreal y1, qreal x2, qreal y2)
{
    // 创建一条线
    QGraphicsLineItem *lineItem = new QGraphicsLineItem();
    lineItem->setLine(QLineF(x1, y1, x2, y2));
    lineItem->setPen(QPen(Qt::red, 2));  // 设置线条颜色和宽度
    scene->addItem(lineItem);
    redlines.append(lineItem);
}
/*保存点和线到Json*/
void Widget::savePointsToJson()
{
    QJsonObject docObject;
    QJsonArray pointsJsonArray;

    // 保存点数据
    for (QGraphicsEllipseItem *point : points)
    {
        QJsonObject pointObject;
        pointObject["x"] = point->pos().x();
        pointObject["y"] = point->pos().y();
        QString name = point->data(0).toString();
        pointObject["name"] = name;  // 保存名字

        pointsJsonArray.append(pointObject);
    }

    docObject["points"] = pointsJsonArray;

    // 保存线数据
    QJsonArray linesJsonArray;
    for (QGraphicsLineItem *line : lines)
    {
        QJsonObject lineObject;
        QLineF lineF = line->line();

        // 保存线的起点和终点坐标
        lineObject["x1"] = lineF.p1().x();
        lineObject["y1"] = lineF.p1().y();
        lineObject["x2"] = lineF.p2().x();
        lineObject["y2"] = lineF.p2().y();

        linesJsonArray.append(lineObject);
    }

    docObject["lines"] = linesJsonArray;

    // 保存到文件
    QFile file("C:/Users/wuzir/Documents/GitHub/Graph/points_and_lines.json");
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

/*从Json绘制线和边*/
void Widget:: loadPointsFromJson()
{
    QFile file("C:/Users/wuzir/Documents/GitHub/Graph/points_and_lines.json");
    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray data = file.readAll();
        file.close();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject())
        {
            QJsonObject docObject = doc.object();
            QJsonArray pointsJsonArray = docObject["points"].toArray();
            for (const QJsonValue &value : pointsJsonArray)
            {
                QJsonObject pointObject = value.toObject();
                qreal x = pointObject["x"].toDouble();
                qreal y = pointObject["y"].toDouble();
                QString name = pointObject["name"].toString();  // 获取名字
                qDebug()<<x<<y<<name;
                loadPoint(x, y, name);  // 加载点并传入名字

            }

            QJsonArray linesJsonArray = docObject["lines"].toArray();
            for (const QJsonValue &value : linesJsonArray)
            {
                QJsonObject lineObject = value.toObject();
                qreal x1 = lineObject["x1"].toDouble();
                qreal y1 = lineObject["y1"].toDouble();
                qreal x2 = lineObject["x2"].toDouble();
                qreal y2 = lineObject["y2"].toDouble();

                // 重新绘制线
                loadLine(x1, y1, x2, y2);
            }
        }
    }
    else
    {
        qDebug() << "Failed to open points_and_lines.json for reading.";
    }
}

/*左键点击点之后链接两个点*/
void Widget::clickToConnectPoints(qreal x, qreal y)
{
    const qreal tolerance = 20;

    for (QGraphicsEllipseItem *point : points)
    {
        QPointF pointPos = point->pos();
        qreal distance = QLineF(QPointF(x, y), pointPos).length();
        if (distance <= tolerance)
        {
            if (firstSelectedPoint == nullptr)
            {

                firstSelectedPoint = point;
                qDebug() << "First point selected at:" << pointPos;
                label->setText("选择第二个点");
            }
            else
            {
                if(firstSelectedPoint->pos()==point->pos())
                {
                    QMessageBox::warning(nullptr,"Error","请重新选择");
                    return;
                }
                QGraphicsLineItem *lineItem = new QGraphicsLineItem();
                lineItem->setLine(QLineF(firstSelectedPoint->pos(), point->pos()));
                lineItem->setFlag(QGraphicsItem::ItemIsSelectable);
                lineItem->setPen(QPen(Qt::blue, 2));
                scene->addItem(lineItem);
                qDebug() << "First point selected at:" << firstSelectedPoint;
                qDebug() << "Second point selected at:" << pointPos;
                qDebug() << "Connecting points with line";
                qDebug() << "Its ok";
                lines.append(lineItem);
                QJsonObject lineObject;
                lineObject["x1"] = firstSelectedPoint->pos().x();
                lineObject["y1"] = firstSelectedPoint->pos().y();
                lineObject["x2"] = point->pos().x();
                lineObject["y2"] = point->pos().y();
                linesArray.append(lineObject);
                savePointsToJson();
                if(firstSelectedPoint)
                {
                    firstSelectedPoint = nullptr;
            }
            break;
        }
        }
    }}

/**/
void Widget::clickToAddPoint(qreal x, qreal y)
{
    QString name = QInputDialog::getText(nullptr, "Enter name", "Enter name");
    if(name=="")
    {
        QMessageBox::warning(nullptr,"Error","地点名字不能为空");
        return;
    }
    loadPoint(x, y, name);
    savePoint(x, y, name);
}








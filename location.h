#ifndef LOCATION_H
#define LOCATION_H

#include <QWidget>
#include "qtypes.h"
class Location
{
public:
    Location();
    QList<QPair<qreal,qreal>> poslist;
    void initLocations();

};

#endif // LOCATION_H

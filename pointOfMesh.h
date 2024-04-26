#ifndef POINTOFMESH_H
#define POINTOFMESH_H

#endif // POINTOFMESH_H

#include <QVector>
#include <QPainter>
#include <QString>
#include <QFile>
#include <QDebug>
#include <QXmlStreamReader>
// #include "obstacle.h"

struct pointOfMesh {
    QPoint coord;
    float walkness;
};

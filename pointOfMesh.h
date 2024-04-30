#ifndef POINTOFMESH_H
#define POINTOFMESH_H

// POINTOFMESH_H

#include <QVector>
#include <QPainter>
#include <QString>
#include <QFile>
#include <QDebug>

struct pointOfMesh {
    QPoint coord;
    float walkness;

    pointOfMesh(QPoint p, float w) {
        this->coord = p;
        this->walkness = w;
    }

    bool operator == (const pointOfMesh& point) const {
        return this->coord.x() == point.coord.x() && this->coord.y() == point.coord.y() && this->walkness == point.walkness;
    }
};

#endif

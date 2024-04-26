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
};

#endif

#ifndef POINTOFMESH_H
#define POINTOFMESH_H

// POINTOFMESH_H

#include <QVector>
#include <QPainter>
#include <QString>
#include <QFile>
#include <QDebug>

struct pointOfMesh {
    QPoint meshCoord;
    QPoint realCoord;
    float walkness;

    pointOfMesh() = default;

    pointOfMesh(QPoint m, QPoint p, float w) {
        this->meshCoord = m;
        this->realCoord = p;
        this->walkness = w;
    }

    //pointOfMesh(const pointOfMesh& mesh) {
    //    this->meshCoord = QPoint(mesh.meshCoord);
    //    this->realCoord = QPoint(mesh.realCoord);
    //    this->walkness = mesh.walkness;
    //}

    bool operator == (const pointOfMesh& point) const {
        return this->meshCoord == point.meshCoord;
        //return this->coord.x() == point.coord.x() && this->coord.y() == point.coord.y() && this->walkness == point.walkness;
    }
};

#endif

#ifndef MESHPOINT_H
#define MESHPOINT_H

#include <QVector>
#include <QPainter>
#include <QString>
#include <QFile>
#include <QDebug>

struct MeshPoint {
    QPoint meshCoord;
    QPoint realCoord;
    float walkness;

    MeshPoint() = default;

    MeshPoint(QPoint m, QPoint p, float w) {
        this->meshCoord = m;
        this->realCoord = p;
        this->walkness = w;
    }

    bool operator == (const MeshPoint& point) const {
        return this->meshCoord == point.meshCoord;
    }
};

#endif // MESHPOINT_H

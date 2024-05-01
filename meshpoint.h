#ifndef MESHPOINT_H
#define MESHPOINT_H

#include <QPoint>
#include <QHash>

struct MeshPoint {
    QPoint meshCoord;
    QPoint realCoord;
    double walkness;

    MeshPoint() = default;

    MeshPoint(QPoint m, QPoint p, double w) {
        this->meshCoord = m;
        this->realCoord = p;
        this->walkness = w;
    }

    bool operator == (const MeshPoint& point) const {
        return meshCoord == point.meshCoord;
    }
};

#endif // MESHPOINT_H

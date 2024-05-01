#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <QPolygon>

struct Obstacle {
    QPolygon poly;
    double walkness;

    Obstacle() = default;

    Obstacle(QPolygon p, float w) {
        this->poly = p;
        this->walkness = w;
    }

    bool operator== (const Obstacle& obst) const {
        return poly == obst.poly;
    }
};

#endif // OBSTACLE_H

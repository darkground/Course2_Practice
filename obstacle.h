#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <QPolygon>

struct Obstacle {
    float walkness;
    QPolygon poly;

    Obstacle(QPolygon p, float w) {
        this->poly = p;
        this->walkness = w;
    }
};

#endif // OBSTACLE_H

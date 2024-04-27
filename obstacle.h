#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <QPolygon>

struct Obstacle {
    float walkness;
    QPolygon poly;


    Obstacle(QPolygon p) {
        this->poly = p;
        this->walkness = 0.;
    }

    Obstacle(QPolygon p, float w) {
        this->poly = p;
        this->walkness = w;
    }
};

#endif // OBSTACLE_H

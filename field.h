#ifndef FIELD_H
#define FIELD_H

#include <QVector>
#include <QPainter>
#include <QString>
#include <QFile>
#include <QDebug>
#include <QXmlStreamReader>
#include "obstacle.h"

class Field
{
public:
    Field(unsigned w, unsigned h);

    void load(QString path);
    void save(QString path);
    float getFactor(QPoint point);
    unsigned count();

    void draw(QPainter* painter);

protected:
    unsigned width, height;
    QVector<Obstacle> obstacles;
};

#endif // FIELD_H

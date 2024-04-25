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
    const QColor outline = QColor(26, 26, 26);
    const int outlineWidth = 2;

    const QColor fillEasy = QColor(212, 212, 212);
    const QColor fillHard = QColor(74, 74, 74);

public:
    Field(unsigned w, unsigned h);

    int load(QString path);
    int save(QString path);
    void resize(unsigned w, unsigned h);
    float getFactor(QPoint point);
    unsigned count();

    void draw(QPainter* painter);

    // Algorithm Functions
    void drawGraphOfField(Field MainField);

protected:
    unsigned width, height;
    QVector<Obstacle> obstacles;
};

#endif // FIELD_H

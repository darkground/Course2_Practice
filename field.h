#ifndef FIELD_H
#define FIELD_H

#include <QVector>
#include <QPainter>
#include <QString>
#include <QFile>
#include <QDebug>
#include <QXmlStreamReader>
#include <QtMath>
#include "obstacle.h"
#include "pointOfMesh.h"

class Field
{
    const bool debug = true;
    const bool drawObstacles = false;
    const bool drawPath = true;

    const int cellSize = 32;

    const QColor outline = QColor(26, 26, 26);
    const QColor drawLastPoint = QColor(76, 76, 224);
    const QColor drawBorder = QColor(53, 51, 97);

    const float polyOutlineWidth = 2.f;
    const float pointOutlineWidth = 0.8f;

    const QColor fillStart = QColor(56, 186, 112);
    const QColor fillEnd = QColor(204, 103, 59);
    const QColor fillEasy = QColor(212, 212, 212);
    const QColor fillHard = QColor(74, 74, 74);

public:
    Field(unsigned w, unsigned h);

    int loadMap(QString path);
    int saveMap(QString path);
    void resizeMap(unsigned w, unsigned h);

    void setStart(QPoint p);
    void setEnd(QPoint p);

    bool addDraw(QPoint p);
    void removeDraw();
    QPolygon getDraw();
    void finishDraw(float w);
    void cancelDraw();

    float getFactor(QPoint point);
    bool removeAt(QPoint point);

    unsigned count();

    void draw(QPainter* painter);

    // Algorithm Functions
    void makeMesh();
    pointOfMesh* pointOnMesh(QPoint point);

    pointOfMesh EditedPoint(int& change, pointOfMesh point);
    pointOfMesh ReverseEditedPoint(int& change, pointOfMesh point);
    bool tryFindWay();
    float algorithmThatFindWay(pointOfMesh& start_point, pointOfMesh& finish_point, pointOfMesh current_point, QVector<pointOfMesh>& visitedPoints, QVector<pointOfMesh>& shortestWayPoints, float& shortestWay, float& currentWay, int& change);

protected:
    unsigned width, height;
    std::optional<QPoint> start, end;
    QPolygon drawing;
    QVector<Obstacle> obstacles;
    QVector<pointOfMesh> way;

    QHash<QPoint, pointOfMesh> mesh;
};

#endif // FIELD_H

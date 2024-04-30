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
#include "meshpoint.h"

class Field
{
    const bool debug = true;
    const bool drawObstacles = false;
    const bool drawPath = true;

    const int cellSize = 2;

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

    // Pathfinding Algorithm Functions
    void makeMesh();
    MeshPoint* pointOnMesh(QPoint point);

    QVector<MeshPoint*> neighbors(MeshPoint point);
    float find();
    float dijkstra(MeshPoint* start_point, MeshPoint* finish_point, QVector<MeshPoint>& shortestWayPoints);

protected:
    unsigned width, height;
    std::optional<QPoint> start, end;
    QPolygon drawing;
    QVector<Obstacle> obstacles;
    QVector<MeshPoint> way;

    QHash<QPoint, MeshPoint> mesh;
};

#endif // FIELD_H

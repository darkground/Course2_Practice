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
#include "prioqueue.h"

class Field
{
    const bool debug = false;
    const bool drawObstacles = true;
    const bool drawPath = true;

    const int pathSmoothing = 8;
    const int cellSize = 2;

    const QColor outline = QColor(26, 26, 26);
    const QColor drawPoint = QColor(0, 4, 64);
    const QColor drawLastPoint = QColor(76, 76, 224);
    const QColor drawBorder = QColor(53, 51, 97);
    const QColor drawPathline = QColor(99, 255, 82);

    const float polyOutlineWidth = 2.f;
    const float pointOutlineWidth = 0.8f;

    const QColor fillStart = QColor(56, 186, 112);
    const QColor fillEnd = QColor(204, 103, 59);
    const QColor fillEasy = QColor(212, 212, 212);
    const QColor fillHard = QColor(74, 74, 74);

    const float pointGrabRadius = 6.;
    bool dragging = false;

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

    void startDrag();
    void startDrag(QPoint from);
    bool toDrag(QPoint where);
    void endDrag(bool flag = true);

    float getFactor(QPoint point);
    bool addPointAt(QPoint point);
    bool removePolyAt(QPoint point);
    bool removePointAt(QPoint point);

    unsigned count();

    void draw(QPainter* painter);

    // Pathfinding Algorithm Functions
    void makeMesh();
    MeshPoint* pointOnMesh(QPoint point);

    float find();
    void dijkstra_neighbor(PriorityQueue<MeshPoint*, float>& queue, QHash<QPoint, QPoint>& came_from, QHash<QPoint, float>& cost_so_far, MeshPoint* current, QPoint offset);
    float dijkstra(MeshPoint* start_point, MeshPoint* finish_point, QVector<MeshPoint>& shortestWayPoints);

protected:
    unsigned width, height;
    std::optional<QPoint> start, end;
    QVector<Obstacle> obstacles;
    QVector<MeshPoint> way;
    QHash<QPoint, MeshPoint> mesh;

    QPolygon* dragPoly = 0;
    QPoint* dragPoint = 0;
    QPolygon drawing;
};

#endif // FIELD_H

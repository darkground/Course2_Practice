#ifndef FIELD_H
#define FIELD_H

#include <QVector>
#include <QPainter>
#include <QString>
#include <QFile>
#include <QDebug>
#include <QXmlStreamReader>
#include "obstacle.h"
#include "meshpoint.h"
#include "prioqueue.h"

typedef std::optional<QPoint> Waypoint;

class Field {
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
    bool drawFlag = false;
    bool dragFlag = false;

public:
    Field(unsigned w, unsigned h);
    ~Field();
    void draw(QPainter* painter);

    int loadMap(QString path);
    int saveMap(QString path);
    void resizeMap(unsigned width, unsigned height);
    bool inMap(const QPoint& point);
    double getFactorMap(const QPoint& point);

    void regenMesh();
    MeshPoint* nearestMesh(const QPoint& point);
    MeshPoint* getMesh(const QPoint& point);

    void setStart(QPoint point);
    void setEnd(QPoint point);
    Waypoint getStart();
    Waypoint getEnd();

    void startDraw();
    bool doDraw(QPoint point);
    void undoDraw();
    void endDraw(double walk);
    void stopDraw();
    QPolygon* getDraw();

    void startDrag();
    void beginDrag(QPoint from);
    bool moveDrag(QPoint where);
    void endDrag();
    void stopDrag();

    Obstacle* getObstacle(const QPoint& point);
    bool removeObstacle(const QPoint& point);
    bool removeObstacle(const Obstacle& obstacle);
    void addPointObstacle(const QPoint& point);
    void addPointObstacle(Obstacle& obst, const QPoint& point);
    bool removePointObstacle(const QPoint& point);
    bool removePointObstacle(Obstacle& obst, const QPoint& point);

    float findPath();
    void aStarN(PriorityQueue<MeshPoint*, float>& queue, QHash<QPoint, QPoint>& origins, QHash<QPoint, float>& costs, MeshPoint* current, MeshPoint* finish, QPoint offset);
    float aStarPath(MeshPoint* start, MeshPoint* finish, QVector<MeshPoint>& way);
    void smoothifyPath(QVector<MeshPoint>& vec);

    unsigned polyCount();

protected:
    unsigned width, height;

    Waypoint start, end;
    QVector<Obstacle> obstacles;
    QVector<MeshPoint> way;
    QHash<QPoint, MeshPoint> mesh;

    QPolygon* dragPoly = 0;
    QPoint* dragPoint = 0;
    QPolygon* drawPoly = 0;
};

#endif // FIELD_H

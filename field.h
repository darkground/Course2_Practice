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
public:
    static constexpr QColor outlineObstacle = QColor(26, 26, 26);
    static constexpr QColor easyObstacle = QColor(212, 212, 212);
    static constexpr QColor hardObstacle = QColor(74, 74, 74);

    static constexpr QColor path = QColor(0, 255, 0);

    static constexpr QColor outlineGrid = QColor(0, 0, 0, 50);

    static constexpr QColor pointDraw = QColor(0, 4, 64);
    static constexpr QColor lastPointDraw = QColor(76, 76, 224);
    static constexpr QColor outlineDraw = QColor(53, 51, 97);

    static constexpr QColor fillStart = QColor(56, 186, 112);
    static constexpr QColor fillEnd = QColor(204, 103, 59);

    static constexpr float polyWidth = 2.f;
    static constexpr float pointWidth = 0.8f;

    static constexpr float pointGrabRadius = 6.;

    bool drawFlag = false;

    bool dGrid = false;
    bool dGridOutline = false;
    bool dNoObstacles = false;
    bool dNoPath = false;
    int cellSize = 2;

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

    Obstacle* getObstacle(const QPoint& point);
    QVector<Obstacle>& getObstacles();
    bool removeObstacle(const QPoint& point);
    bool removeObstacle(const Obstacle& obstacle);
    bool removeFromObstacle(const QPoint& point);
    bool removeFromObstacle(Obstacle& obst, const QPoint& point);
    bool addToObstacle(const QPoint& point);
    bool addToObstacle(Obstacle& obst, const QPoint& point);

    float findPath();
    void aStarN(PriorityQueue<MeshPoint*, float>& queue, QHash<QPoint, QPoint>& origins, QHash<QPoint, float>& costs, MeshPoint* current, MeshPoint* finish, QPoint offset);
    float aStarPath(MeshPoint* start, MeshPoint* finish, QVector<MeshPoint>& way);
    QVector<MeshPoint> smoothv1Path(const QVector<MeshPoint>& vec);
    QVector<MeshPoint> smoothv2Path(const QVector<MeshPoint>& vec, int maxSteps = 16);
    QVector<MeshPoint> splicePath(const QVector<MeshPoint>& vec, int interval = 2);

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

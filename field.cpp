#include "field.h"
#include "utils.h"

Field::Field(unsigned w, unsigned h) {
    this->width = w;
    this->height = h;
    regenMesh();
}

Field::~Field() {
    if (drawPoly != 0) delete drawPoly;
}

void Field::draw(QPainter* painter) {
    QPen p;

    if (debug) {
        p.setColor(QColor(0, 0, 0, 50));
        painter->setPen(p);
        for (auto it = mesh.keyValueBegin(); it != mesh.keyValueEnd(); ++it) {
            QPoint startPoint = it->second.realCoord;
            QColor sqColor = mix(Field::fillEasy, Field::fillHard, it->second.walkness);
            painter->setBrush(sqColor);
            painter->drawRect(startPoint.x(), startPoint.y(), Field::cellSize, Field::cellSize);
        }
    }

    p.setWidth(Field::polyOutlineWidth);
    p.setColor(Field::outline);
    painter->setPen(p);

    if (drawObstacles) {
        for (Obstacle& obst : obstacles) {
            QColor polyColor = mix(Field::fillEasy, Field::fillHard, obst.walkness);
            painter->setBrush(polyColor);
            painter->drawPolygon(obst.poly);
        }
    }

    if (drawPath) {
        p.setColor(Field::drawPathline);
        painter->setPen(p);
        for (int i = 1; i * Field::pathSmoothing < way.length(); i++) {
            MeshPoint& prev = way[(i-1) * Field::pathSmoothing];
            MeshPoint& next = way[i * Field::pathSmoothing];
            painter->drawLine(prev.realCoord, next.realCoord);
        }
    }

    p.setColor(Field::outline);
    painter->setPen(p);

    if (drawFlag) {
        p.setColor(Field::drawBorder);
        painter->setPen(p);
        painter->setBrush(Field::fillEasy);
        painter->drawPolygon(*drawPoly);
        painter->setBrush(QColor(0, 0, 0, 0));
        for (QPoint& point : *drawPoly) {
            p.setColor(point == drawPoly->last() ? Field::drawLastPoint : Field::drawPoint);
            painter->setPen(p);
            painter->drawEllipse(point, 6, 6);
        }
    }

    if (dragFlag) {
        painter->setBrush(QColor(0, 0, 0, 0));
        for (Obstacle& o : obstacles) {
            for (QPoint& point : o.poly) {
                p.setColor(&point == dragPoint ? Field::drawLastPoint : Field::drawPoint);
                painter->setPen(p);
                painter->drawEllipse(point, 6, 6);
            }
        }
    }

    p.setWidth(Field::pointOutlineWidth);
    painter->setPen(p);
    if (start.has_value()) {
        painter->setBrush(Field::fillStart);
        painter->drawEllipse(*start, 6, 6);
    }
    if (end.has_value()) {
        painter->setBrush(Field::fillEnd);
        painter->drawEllipse(*end, 6, 6);
    }
}

// Map

int Field::loadMap(QString path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return -1;
    QXmlStreamReader xml(&file);
    bool ok;
    while (!xml.atEnd() && !xml.hasError())
    {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartElement)
        {
            if (xml.name() == QString("polygons")) {
                while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == QString("polygons"))) {
                    if (xml.readNext() == QXmlStreamReader::StartElement && xml.name() == QString("poly")) {
                        QXmlStreamAttributes polyAttrs = xml.attributes();
                        if (!polyAttrs.hasAttribute("walkness")) return -2;
                        QPolygon poly;
                        double w = polyAttrs.value("walkness").toDouble(&ok);
                        if (!ok || (w < 0. || w > 1.)) return -3;
                        while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == QString("poly"))) {
                            if (xml.readNext() == QXmlStreamReader::StartElement && xml.name() == QString("point")) {
                                QXmlStreamAttributes pointAttrs = xml.attributes();
                                if (!pointAttrs.hasAttribute("x") || !pointAttrs.hasAttribute("y")) return -2;
                                unsigned x = pointAttrs.value("x").toInt(&ok);
                                if (!ok || (x < 0 || x > width)) return -3;
                                unsigned y = pointAttrs.value("y").toInt(&ok);
                                if (!ok || (y < 0 || y > height)) return -3;
                                poly << QPoint(x, y);
                            }
                        }
                        obstacles.append(Obstacle(poly, w));
                    }
                }
            }
            if (xml.name() == QString("path"))
                continue; //todo
        }
    }
    return 0;
}

int Field::saveMap(QString path) {
    return 0;
}

void Field::resizeMap(unsigned width, unsigned height) {
    obstacles.clear();
    this->width = width;
    this->height = height;
    qDebug() << "Field::size" << "Set to" << width << height;
    regenMesh();
}

bool Field::inMap(const QPoint& point) {
    return point.x() >= 0 && point.y() >= 0 && point.x() < (int)width && point.y() < (int)height;
}

double Field::getFactorMap(const QPoint& point) {
    for (Obstacle& obst : obstacles) {
        if (obst.poly.containsPoint(point, Qt::FillRule::OddEvenFill)) {
            return obst.walkness;
        }
    }
    return 0.;
}

// Mesh

void Field::regenMesh() {
    mesh.clear();

    for (unsigned i = 0; i * Field::cellSize < width; i++) {
        for (unsigned j = 0; j * Field::cellSize < height; j++) {
            QPoint meshCoord = QPoint(i, j);
            QPoint realCoord = QPoint(i * Field::cellSize, j * Field::cellSize);
            double factor = getFactorMap(realCoord);
            MeshPoint meshp(meshCoord, realCoord, factor);

            mesh[meshCoord] = meshp;
        }
    }

    qInfo() << "Field::mesh" << "Generated size" << mesh.size();
}

MeshPoint* Field::nearestMesh(const QPoint& point) {
    MeshPoint* shortestMesh = 0;
    float shortestDist = width * height;
    for (auto it = mesh.keyValueBegin(); it != mesh.keyValueEnd(); ++it) {
        MeshPoint& meshp = it->second;
        float dist = euclideanDistance(meshp.realCoord, point);
        if (dist < shortestDist) {
            shortestDist = dist;
            shortestMesh = &meshp;
        }
    }
    return shortestMesh;
}

MeshPoint* Field::getMesh(const QPoint& point) {
    if (mesh.contains(point)) return &mesh[point];
    return 0;
}

// Points

void Field::setStart(QPoint point) {
    qInfo() << "Field::start" << point;
    start.emplace(point);
}

void Field::setEnd(QPoint point) {
    qInfo() << "Field::end" << point;
    end.emplace(point);
}

Waypoint Field::getStart() {
    return start;
}

Waypoint Field::getEnd() {
    return end;
}

// Polygon Drawing

void Field::startDraw() {
    qInfo() << "Field::polyDraw" << "Start";
    drawFlag = true;
    drawPoly = new QPolygon();
}

bool Field::doDraw(QPoint p) {
    if (!drawFlag) return false;
    if (getFactorMap(p) != 0.) return false;
    QPolygon poly(*drawPoly);
    poly << p;
    for (Obstacle& o : obstacles) {
        if (o.poly.intersects(poly)) return false;
    }
    qInfo() << "Field::polyDraw" << "Append" << p;
    (*drawPoly) << p;
    return true;
}

void Field::undoDraw() {
    if (!drawFlag) return;
    if (!drawPoly->empty()) {
        qInfo() << "Field::polyDraw" << "Undo";
        drawPoly->removeLast();
    }
}

QPolygon* Field::getDraw() {
    return drawPoly;
}

void Field::endDraw(double w) {
    if (!drawFlag) return;
    qInfo() << "Field::polyDraw" << "End, W =" << w;
    obstacles.append(Obstacle(*drawPoly, w));
    regenMesh();
    stopDraw();
}

void Field::stopDraw() {
    qInfo() << "Field::polyDraw" << "Stop";
    drawFlag = false;
    delete drawPoly;
    drawPoly = 0;
}

// Polygon Editing

void Field::startDrag() {
    qInfo() << "Field::polyDrag" << "Start";
    dragFlag = true;
}

void Field::beginDrag(QPoint from) {
    dragPoly = 0;
    dragPoint = 0;
    float closest = Field::pointGrabRadius;
    for (Obstacle& obst : obstacles) {
        for (QPoint& vertex : obst.poly) {
            float dst = euclideanDistance(from, vertex);
            if (dst < closest && dst <= Field::pointGrabRadius) {
                dragPoly = &obst.poly;
                dragPoint = &vertex;
                closest = dst;
            }
        }
    }
    if (dragPoint != 0) qInfo() << "Field::polyDrag" << "Attach" << *dragPoint;
    else qInfo() << "Field::polyDrag" << "Attach" << "NULL";
}

bool Field::moveDrag(QPoint where) {
    if (dragPoint == 0) return false;
    if (!inMap(where)) return false;
    QPolygon it = *dragPoly;
    QPoint old = *dragPoint;

    *dragPoint = where;

    for (Obstacle& obst : obstacles) {
        if (obst.poly != it && obst.poly.intersects(it)) {
            *dragPoint = old;
            return false;
        }
    }
    return true;
}

void Field::endDrag() {
    dragPoly = 0;
    dragPoint = 0;
    qInfo() << "Field::polyDrag" << "End";
}

void Field::stopDrag() {
    dragFlag = false;
    regenMesh();
    qInfo() << "Field::polyDrag" << "Stop";
}

Obstacle* Field::getObstacle(const QPoint& point) {
    for (Obstacle& obst : obstacles) {
        if (obst.poly.containsPoint(point, Qt::FillRule::OddEvenFill)) {
            return &obst;
        }
    }
    return 0;
}

bool Field::removeObstacle(const QPoint& point) {
    Obstacle* obst = getObstacle(point);
    if (obst == 0) return false;
    return removeObstacle(*obst);
}

bool Field::removeObstacle(const Obstacle& obst) {
    qInfo() << "Field::remObst" << obst.walkness;
    if (obstacles.removeOne(obst)) {
        regenMesh();
        return true;
    }
    return false;
}

void Field::addPointObstacle(const QPoint& point) {
    Obstacle* obst = getObstacle(point);
    if (obst == 0) return;
    addPointObstacle(*obst, point);
}

void Field::addPointObstacle(Obstacle& obst, const QPoint& point) {
    QVector<QPoint> points = obst.poly.toVector();
    points.append(obst.poly.first());
    float idx = 0;
    float closest = width * height;

    for (int i = 1; i < points.size(); i++) {
        QLine line(points[i-1], points[i]);
        QPoint nearest = nearestPointOnLine(line, point);
        float dst = euclideanDistance(nearest, point);
        if (dst < closest) {
            idx = i;
            closest = dst;
        }
    }
    obst.poly.insert(idx, point);
    qInfo() << "Field::addPoint" << point;
}

bool Field::removePointObstacle(const QPoint& point) {
    Obstacle* obst = getObstacle(point);
    if (obst == 0) return false;
    return removePointObstacle(*obst, point);
}

bool Field::removePointObstacle(Obstacle& obst, const QPoint& point) {
    int idx = -1;
    float closest = width * height;

    for (int i = 0; i < obst.poly.size(); i++) {
        QPoint& pt = obst.poly[i];
        float dst = euclideanDistance(pt, point);
        if (dst < closest && dst <= Field::pointGrabRadius) {
            idx = i;
            closest = dst;
        }
    }
    if (idx == -1) return false;
    obst.poly.removeAt(idx);
    qInfo() << "Field::remPoint" << point;
    if (obst.poly.size() < 3) removeObstacle(obst);
    return true;
}

// Pathfinding

float Field::findPath() {
    if (!start.has_value() || !end.has_value()) return -1;
    MeshPoint* mstart = nearestMesh(*start);
    MeshPoint* mend = nearestMesh(*end);
    if (mstart == 0 || mend == 0) return false;

    way.clear();
    float shortest = aStarPath(mstart, mend, way);
    qInfo() << "Field::find" << shortest << "/" << way.length();
    return shortest;
}

void Field::aStarN(
    PriorityQueue<MeshPoint*, float>& queue,
    QHash<QPoint, QPoint>& origins,
    QHash<QPoint, float>& costs,
    MeshPoint* current,
    MeshPoint* finish,
    QPoint offset
    ) {
    QPoint off = current->meshCoord + offset;
    if (!mesh.contains(off)) return;
    MeshPoint* neighbor = &mesh[off];
    if (neighbor->walkness >= 1.) return;
    float new_cost = costs[current->meshCoord] + 1 + neighbor->walkness;
    if (!costs.contains(neighbor->meshCoord) || new_cost < costs[neighbor->meshCoord]) {
        costs[neighbor->meshCoord] = new_cost;
        origins[neighbor->meshCoord] = current->meshCoord;
        queue.put(neighbor, new_cost + simpleDistance(neighbor->meshCoord, finish->meshCoord));
    }
}

// Алгоритм поиска пути A*
float Field::aStarPath(MeshPoint* start, MeshPoint* finish, QVector<MeshPoint>& way) {
    way.clear();
    PriorityQueue<MeshPoint*, float> queue;
    queue.put(start, 1.);

    QHash<QPoint, QPoint> origins;
    QHash<QPoint, float> costs;
    origins[start->meshCoord] = start->meshCoord;
    costs[start->meshCoord] = 1.;
    
    while (!queue.empty()) {
        MeshPoint* current = queue.get();
        if (current == finish) break;
        
        aStarN(queue, origins, costs, current, finish, QPoint(1, 0));
        aStarN(queue, origins, costs, current, finish, QPoint(0, 1));
        aStarN(queue, origins, costs, current, finish, QPoint(-1, 0));
        aStarN(queue, origins, costs, current, finish, QPoint(0, -1));
    }

    QPoint current = finish->meshCoord;
    if (!origins.contains(current)) return 0;
    float cost = costs[finish->meshCoord];
    while (current != start->meshCoord) {
        MeshPoint meshp = mesh[current];
        way.append(meshp);
        current = origins[current];
    }
    way.append(*start);
    std::reverse(way.begin(), way.end());
    return cost;
}

unsigned Field::polyCount() {
    return obstacles.length();
}

void Field::smoothifyPath(QVector<MeshPoint>& vec) {

    // ...
}

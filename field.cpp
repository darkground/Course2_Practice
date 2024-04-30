#include "field.h"
#include "prioqueue.h"

Field::Field(unsigned w, unsigned h) {
    this->width = w;
    this->height = h;
    makeMesh();
}

QColor mix(QColor c1, QColor c2, float factor) {
    return QColor(
        c1.red() * (1 - factor) + c2.red(),
        c1.green() * (1 - factor) + c2.green(),
        c1.blue() * (1 - factor) + c2.blue()
    );
}

// getPath...

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
                        float w = polyAttrs.value("walkness").toFloat(&ok);
                        if (!ok || (w < 0. || w > 1.)) return -3;
                        while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == QString("poly"))) {
                            if (xml.readNext() == QXmlStreamReader::StartElement && xml.name() == QString("point")) {
                                QXmlStreamAttributes pointAttrs = xml.attributes();
                                if (!pointAttrs.hasAttribute("x") || !pointAttrs.hasAttribute("y")) return -2;
                                unsigned x = pointAttrs.value("x").toInt(&ok);
                                if (!ok || (x < 0 || x > this->width)) return -3;
                                unsigned y = pointAttrs.value("y").toInt(&ok);
                                if (!ok || (y < 0 || y > this->height)) return -3;
                                poly << QPoint(x, y);
                            }
                        }
                        this->obstacles.append(Obstacle(poly, w));
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

void Field::resizeMap(unsigned w, unsigned h) {
    this->obstacles.clear();
    this->width = w;
    this->height = h;
    qDebug() << "Resized to" << w << h;
    makeMesh();
}

unsigned Field::count() {
    return this->obstacles.length();
}

void Field::draw(QPainter* painter) {
    QPen p;

    if (debug) {
        p.setColor(QColor(0, 0, 0, 0));
        painter->setPen(p);
        for (auto it = this->mesh.keyValueBegin(); it != this->mesh.keyValueEnd(); ++it) {
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
        for (Obstacle& obst : this->obstacles) {
            QColor polyColor = mix(Field::fillEasy, Field::fillHard, obst.walkness);
            painter->setBrush(polyColor);
            painter->drawPolygon(obst.poly);
        }
    }

    if (drawPath && !way.empty()) {
        p.setColor(Field::drawBorder);
        painter->setPen(p);
        for (int i = 1; i < way.length(); i++) {
            MeshPoint& prev = way[i-1];
            MeshPoint& next = way[i];
            painter->drawLine(prev.realCoord, next.realCoord);
        }
    }

    if (!this->drawing.empty()) {
        p.setColor(Field::drawBorder);
        painter->setPen(p);
        painter->setBrush(Field::fillEasy);
        painter->drawPolygon(this->drawing);
        p.setColor(Field::drawLastPoint);
        painter->setPen(p);
        painter->setBrush(QColor(0, 0, 0, 0));
        painter->drawEllipse(this->drawing.last(), 6, 6);
    }

    p.setWidth(Field::pointOutlineWidth);
    painter->setPen(p);
    if (this->start.has_value()) {
        painter->setBrush(Field::fillStart);
        painter->drawEllipse(this->start.value(), 4, 4);
    }
    if (this->end.has_value()) {
        painter->setBrush(Field::fillEnd);
        painter->drawEllipse(this->end.value(), 4, 4);
    }
}

float Field::getFactor(QPoint point) {
    for (Obstacle& obst : this->obstacles) {
        if (obst.poly.containsPoint(point, Qt::FillRule::OddEvenFill)) {
            return obst.walkness;
        }
    }
    return 0.;
}

void Field::setStart(QPoint s) {
    this->start.emplace(s);
}

void Field::setEnd(QPoint s) {
    this->end.emplace(s);
}

bool Field::addDraw(QPoint p) {
    if (!this->drawing.empty()) {
        QPolygon poly(this->drawing);
        poly << p;
        for (Obstacle& o : this->obstacles) {
            if (o.poly.intersects(poly)) return false;
        }
    }
    if (getFactor(p) != 0.) return false;
    this->drawing << p;
    return true;
}

void Field::removeDraw() {
    if (!this->drawing.empty()) {
        this->makeMesh();
        this->drawing.pop_back();
    }
}

QPolygon Field::getDraw() {
    return this->drawing;
}

void Field::finishDraw(float w) {
    this->obstacles.append(Obstacle(this->drawing, w));
    this->drawing.clear();
    this->makeMesh();
}

void Field::cancelDraw() {
    this->drawing.clear();
}

bool Field::removeAt(QPoint point) {
    int idx = -1;
    for (int i = 0; i < this->obstacles.count(); i++) {
        if (this->obstacles[i].poly.containsPoint(point, Qt::FillRule::OddEvenFill)) {
            idx = i;
            break;
        }
    }
    if (idx == -1) return false;
    this->obstacles.removeAt(idx);
    return true;
}

void Field::makeMesh() {
    qDebug() << "Creating mesh" << this->width << this->height;
    this->mesh.clear();

    // Создание списка из всех точек поля
    for(unsigned i = 0; i * Field::cellSize < this->width; i++) {
        for(unsigned j = 0; j * Field::cellSize < this->height; j++) {
            QPoint meshCoord = QPoint(i, j);
            QPoint realCoord = QPoint(i * Field::cellSize, j * Field::cellSize);
            float factor = getFactor(realCoord);
            MeshPoint mesh(meshCoord, realCoord, factor);

            this->mesh[meshCoord] = mesh;
        }
    }

    qDebug() << "Mesh size" << this->mesh.size();
}

QVector<MeshPoint*> Field::neighbors(MeshPoint point) {
    QVector<MeshPoint*> nei;
    if (mesh.contains(QPoint(point.meshCoord.x()+1, point.meshCoord.y()))) nei.append(&mesh[QPoint(point.meshCoord.x()+1, point.meshCoord.y())]);
    if (mesh.contains(QPoint(point.meshCoord.x(), point.meshCoord.y()+1))) nei.append(&mesh[QPoint(point.meshCoord.x(), point.meshCoord.y()+1)]);
    if (mesh.contains(QPoint(point.meshCoord.x()-1, point.meshCoord.y()))) nei.append(&mesh[QPoint(point.meshCoord.x()-1, point.meshCoord.y())]);
    if (mesh.contains(QPoint(point.meshCoord.x(), point.meshCoord.y()-1))) nei.append(&mesh[QPoint(point.meshCoord.x(), point.meshCoord.y()-1)]);
    return nei;
}

MeshPoint* Field::pointOnMesh(QPoint point) {
    MeshPoint* shortestMesh = 0;
    float shortestDist = this->width * this->height;
    for (auto it = this->mesh.keyValueBegin(); it != this->mesh.keyValueEnd(); ++it) {
        MeshPoint& mesh = it->second;
        float dist = qSqrt(qPow(mesh.realCoord.x() - point.x(), 2) + qPow(mesh.realCoord.y() - point.y(), 2));
        if (dist < shortestDist) {
            shortestDist = dist;
            shortestMesh = &mesh;
        }
    }
    return shortestMesh;
}

float Field::find() {
    if (!this->start.has_value() || !this->end.has_value()) return -1;
    MeshPoint* start = pointOnMesh(this->start.value());
    MeshPoint* end = pointOnMesh(this->end.value());
    if (start == 0 || end == 0) return false;

    this->way.clear();
    float shortest = dijkstra(start, end, this->way);
    return shortest;
}

// Алгоритм поиска пути Дейкстры
float Field::dijkstra(MeshPoint* start_point, MeshPoint* finish_point, QVector<MeshPoint>& shortestWay) {
    shortestWay.clear();
    PriorityQueue<MeshPoint*, float> queue;
    queue.put(start_point, 1.);

    QHash<QPoint, QPoint> came_from;
    QHash<QPoint, float> cost_so_far;
    came_from[start_point->meshCoord] = start_point->meshCoord;
    cost_so_far[start_point->meshCoord] = 1.;
    
    while (!queue.empty()) {
        MeshPoint* current = queue.get();
        if (current == finish_point) break;

        for (MeshPoint* neighbor : neighbors(*current)) {
            if (neighbor->walkness < 1.) {
                float new_cost = cost_so_far[current->meshCoord] + 1 + neighbor->walkness;
                if (!cost_so_far.contains(neighbor->meshCoord) || new_cost < cost_so_far[neighbor->meshCoord]) {
                    cost_so_far[neighbor->meshCoord] = new_cost;
                    came_from[neighbor->meshCoord] = current->meshCoord;
                    queue.put(neighbor, new_cost);
                }
            }
        }
    }

    QPoint current = finish_point->meshCoord;
    if (!came_from.contains(current)) return 0;
    float cost = cost_so_far[finish_point->meshCoord];
    while (current != start_point->meshCoord) {
        MeshPoint mesh = this->mesh[current];
        shortestWay.append(mesh);
        current = came_from[current];
    }
    shortestWay.append(*start_point);
    std::reverse(shortestWay.begin(), shortestWay.end());
    return cost;
}

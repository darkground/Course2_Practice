#include "field.h"
#include "pointOfMesh.h"
#include <QVector>

Field::Field(unsigned w, unsigned h) {
    this->width = w;
    this->height = h;
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
    makeMeshOfField();
}

unsigned Field::count() {
    return this->obstacles.length();
}

void Field::draw(QPainter* painter) {
    QPen p;

    if (debug) {
        p.setWidth(1);
        p.setColor(Qt::red);
        painter->setBrush(Qt::yellow);
        painter->setPen(p);
        for (pointOfMesh& meshp : this->meshPoints) {
            QPoint startPoint = meshp.coord;
            QPoint endPoint = meshp.coord + QPoint(this->cellSize, this->cellSize);
            painter->drawRect(startPoint.x(), startPoint.y(), endPoint.x(), endPoint.y());
        }
    }

    p.setWidth(Field::polyOutlineWidth);
    p.setColor(Field::outline);
    painter->setPen(p);

    for (Obstacle& obst : this->obstacles) {
        QColor polyColor = mix(Field::fillEasy, Field::fillHard, obst.walkness);
        painter->setBrush(polyColor);
        painter->drawPolygon(obst.poly);
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
        this->drawing.pop_back();
    }
}

QPolygon Field::getDraw() {
    return this->drawing;
}

void Field::finishDraw(float w) {
    this->obstacles.append(Obstacle(this->drawing, w));
    this->drawing.clear();
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

// test commit

void Field::makeMeshOfField() {
    this->meshPoints.clear();

    // Создание списка из всех точек поля
    for(unsigned i = 0; i < this->width; i += Field::cellSize) {
        for(unsigned j = 0; j < this->height; j += Field::cellSize) {
            QPoint point = QPoint(i, j);
            float factor = getFactor(point);
            pointOfMesh meshPoint(point, factor);

            this->meshPoints.append(meshPoint);
        }
    }

    // Присваивание точкам поля проходимости
    //for(unsigned i = 0; i < MainField.width * MainField.height; ++i) {
    //    list_of_points[i].walkness = getFactor(list_of_points[i].coord);
    //}
}

pointOfMesh& Field::EditPoint(int& change, pointOfMesh& point) {
    // 1 - увеличить x на 1, 2 - уменьшить x на 1, 3 - увеличить у на 1, 4 - уменьшить у на 1
    if(change == 1) {
        point.coord.rx()++;
    }
    else if(change == 2) {
        point.coord.rx()--;
    }
    else if(change == 3) {
        point.coord.ry()++;
    }
    else if(change == 4) {
        point.coord.ry()--;
    }
    return point;
}

pointOfMesh& Field::ReverseEditPoint(int& change, pointOfMesh& point) {
    // 2 - увеличить x на 1, 1 - уменьшить x на 1, 4 - увеличить у на 1, 3 - уменьшить у на 1
    if(change == 1) {
        point.coord.rx()--;
    }
    else if(change == 2) {
        point.coord.rx()++;
    }
    else if(change == 3) {
        point.coord.ry()--;
    }
    else if(change == 4) {
        point.coord.ry()++;
    }
    return point;
}

// Алгоритм поиска пути
int Field::algorithmThatFindWay(pointOfMesh& start_point, pointOfMesh& finish_point, pointOfMesh& current_point,  QVector <pointOfMesh> visitedPoints, QVector <pointOfMesh> shortestWayPoints, int& shortestWay, int& currentWay, int& change) {
    if(this->meshPoints.contains(current_point)){ // Проверка, существует ли точка
        if(current_point.walkness == 1) {
            current_point = ReverseEditPoint(change, current_point);
            return 0;
        }
        if(currentWay > shortestWay && shortestWay != 1) {
            current_point = ReverseEditPoint(change, current_point);
            return 0;
        }
        visitedPoints.append(current_point);
        currentWay += current_point.walkness;
        if(current_point == finish_point) {
            if(shortestWay != -1 || currentWay < shortestWay) { // Если до этого момента не было найдено пути или новый путь короче, то заменяем кратчайший на текущий
                shortestWay = currentWay;
                currentWay -= current_point.walkness;
                visitedPoints.pop_back();
                current_point = ReverseEditPoint(change, current_point);
                return 0;
            }
        }
        change = 1;
        algorithmThatFindWay(start_point, finish_point, EditPoint(change, current_point), visitedPoints, shortestWayPoints, shortestWay, currentWay, change);
        change = 2;
        algorithmThatFindWay(start_point, finish_point, EditPoint(change, current_point), visitedPoints, shortestWayPoints, shortestWay, currentWay, change);
        change = 3;
        algorithmThatFindWay(start_point, finish_point, EditPoint(change, current_point), visitedPoints, shortestWayPoints, shortestWay, currentWay, change);
        change = 4;
        algorithmThatFindWay(start_point, finish_point, EditPoint(change, current_point), visitedPoints, shortestWayPoints, shortestWay, currentWay, change);
    }
    else {
        current_point = ReverseEditPoint(change, current_point);
        return 0;
    }
    return 0;
}

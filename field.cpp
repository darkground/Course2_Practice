#include "field.h"

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
}

unsigned Field::count() {
    return this->obstacles.length();
}

void Field::draw(QPainter* painter) {
    QPen p(Field::outline, Field::polyOutlineWidth);
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
        painter->setBrush(QColor(0, 0, 0, 0));
        for (QPoint& point : this->drawing) {
            if (point == this->drawing.last()) {
                p.setColor(Field::drawLastPoint);
            } else {
                p.setColor(Field::drawPoint);
            }
            painter->setPen(p);
            painter->drawEllipse(point, 6, 6);
        }
    }

    if (this->dragging) {
        painter->setBrush(QColor(0, 0, 0, 0));
        for (Obstacle& o : this->obstacles) {
            for (QPoint& point : o.poly) {
                if (&point == this->dragPoint) {
                    p.setColor(Field::drawLastPoint);
                } else {
                    p.setColor(Field::drawPoint);
                }
                painter->setPen(p);
                painter->drawEllipse(point, 6, 6);
            }
        }
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

void Field::startDrag() {
    this->dragging = true;
}

void Field::startDrag(QPoint from) {
    startDrag();
    QPolygon* poly = 0;
    QPoint* point = 0;
    float closest = Field::pointGrabRadius;
    for (Obstacle& obst : this->obstacles) {
        for (QPoint& p : obst.poly) {
            float dst = qSqrt(qPow(from.x() - p.x(), 2) + qPow(from.y() - p.y(), 2));
            if (dst < closest && dst <= Field::pointGrabRadius) {
                point = &p;
                poly = &obst.poly;
                closest = dst;
            }
        }
    }
    this->dragPoly = poly;
    this->dragPoint = point;
}

bool Field::toDrag(QPoint where) {
    if (this->dragPoint != 0) {
        QPolygon it = *this->dragPoly;
        QPoint old = *this->dragPoint;

        if (where.x() < 0 || where.y() < 0 || where.x() >= (int)this->width || where.y() >= (int)this->height) return false;
        *(this->dragPoint) = where;

        for (Obstacle& o : this->obstacles) {
            if (o.poly != it && o.poly.intersects(it)) {
                *(this->dragPoint) = old;
                return false;
            }
        }
        return true;
    }
    return false;
}

void Field::endDrag(bool flag) {
    this->dragPoly = 0;
    this->dragPoint = 0;
    if (flag) this->dragging = false;
}


bool Field::removePolyAt(QPoint point) {
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

bool Field::removePointAt(QPoint point) {
    QPolygon* poly = 0;
    int idx = -1;
    float closest = Field::pointGrabRadius;
    for (Obstacle& obst : this->obstacles) {
        for (int i = 0; i < obst.poly.size(); i++) {
            QPoint& pt = obst.poly[i];
            float dst = qSqrt(qPow(pt.x() - point.x(), 2) + qPow(pt.y() - point.y(), 2));
            if (dst < closest && dst <= Field::pointGrabRadius) {
                idx = i;
                poly = &obst.poly;
                closest = dst;
            }
        }
    }
    if (idx == -1) return false;
    poly->removeAt(idx);
    if (poly->size() < 3) {
        for (int i = 0; i < this->obstacles.count(); i++) {
            if (&this->obstacles[i].poly == poly) {
                this->obstacles.removeAt(i);
                break;
            }
        }
    }
    return true;
}

QPoint nearestPoint(QLine& liner, QPoint& pointr){
    double APx = pointr.x() - liner.x1();
    double APy = pointr.y() - liner.y1();
    double ABx = liner.x2() - liner.x1();
    double ABy = liner.y2() - liner.y1();
    double magAB2 = ABx*ABx + ABy*ABy;
    double ABdotAP = ABx*APx + ABy*APy;
    double t = ABdotAP / magAB2;

    QPoint newPoint;

    if (t < 0) {
        newPoint = liner.p1();
    }else if (t > 1){
        newPoint = liner.p2();
    }else{
        newPoint.setX(liner.x1() + ABx*t);
        newPoint.setY(liner.y1() + ABy*t);
    }

    return newPoint;
}

bool Field::addPointAt(QPoint point) {
    QPolygon* poly = 0;
    int idx = -1;
    float closest = this->width * this->height;
    for (Obstacle& obst : this->obstacles) {
        if (obst.poly.containsPoint(point, Qt::OddEvenFill)) {
            QVector<QPoint> points = obst.poly.toVector();
            points.append(obst.poly.first());
            for (int i = 1; i < points.size(); i++) {
                QLine line(points[i-1], points[i]);
                QPoint nearest = nearestPoint(line, point);
                float dst = qSqrt(qPow(nearest.x() - point.x(), 2) + qPow(nearest.y() - point.y(), 2));
                if (dst < closest) {
                    idx = i;
                    poly = &obst.poly;
                    closest = dst;
                }
            }
            break;
        }
    }
    if (idx == -1) return false;
    poly->insert(idx, point);
    return true;
}

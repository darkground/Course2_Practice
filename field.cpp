#include "field.h"
#include "pointOfMesh.h"

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

int Field::load(QString path) {
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

int Field::save(QString path) {
    return 0;
}

void Field::resize(unsigned w, unsigned h) {
    this->obstacles.clear();
    this->width = w;
    this->height = h;
}

unsigned Field::count() {
    return this->obstacles.length();
}

void Field::draw(QPainter* painter) {
    QPen p(Field::outline);
    p.setWidth(Field::outlineWidth);
    painter->setPen(p);
    painter->setBrush(Qt::green);

    for (Obstacle& obst : this->obstacles) {
        QColor polyColor = mix(Field::fillEasy, Field::fillHard, obst.walkness);
        painter->setBrush(polyColor);
        painter->drawPolygon(obst.poly);
    }
}

float Field::getFactor(QPoint point) {
    for (Obstacle& obst : this->obstacles) {
        if (obst.poly.containsPoint(point, Qt::FillRule::OddEvenFill)) {
            return obst.walkness;
        }
    }
    return 1.;
}

// test commit

void Field::drawMeshOfField(Field MainField) {
    QVector <pointOfMesh> list_of_points(0);
    //int index = 0;

    // Создание списка из всех точек поля
    for(int i = 0; i < MainField.height; ++i) {
        for(int j = 0; j < MainField.width; ++j) {
            pointOfMesh point;
            QPoint coords(i, j);
            point.coord = coords;

            list_of_points.append(point);
            //++index;
        }
    }

    // Присваивание точкам поля проходимости
    for(int i = 0; i < MainField.width * MainField.height; ++i) {
        list_of_points[i].walkness = getFactor(list_of_points[i].coord);
    }
}

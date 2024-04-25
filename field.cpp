#include "field.h"

Field::Field(unsigned w, unsigned h) {
    this->width = w;
    this->height = h;
}

QVector<Obstacle> getObstacles(QXmlStreamReader* xml) {
    QVector<Obstacle> v;
    while (!xml->atEnd() && !xml->hasError())
    {
        QXmlStreamReader::TokenType token = xml->readNext();
        if (token == QXmlStreamReader::StartElement && xml->name() == QString("poly"))
        {
            QPolygon poly;
            float w;
            QXmlStreamAttributes polyAttrs = xml->attributes();
            if (!polyAttrs.hasAttribute("walkness"))
                return QVector<Obstacle>();
            w = polyAttrs.value("walkness").toFloat();
            qDebug() << "Polygon" << v.length() << "starts; w =" << w;
            while (!(xml->tokenType() == QXmlStreamReader::EndElement && xml->name() == QString("poly")))
            {
                if (xml->readNext() == QXmlStreamReader::StartElement && xml->name() == QString("point"))
                {
                    QPoint p;
                    QXmlStreamAttributes pointAttrs = xml->attributes();
                    if (!pointAttrs.hasAttribute("x") || !pointAttrs.hasAttribute("y"))
                        return QVector<Obstacle>();
                    p.setX(pointAttrs.value("x").toInt());
                    p.setY(pointAttrs.value("y").toInt());
                    qDebug() << "Point" << p.x() << "," << p.y();
                    poly << p;
                }
            }
            qDebug() << "Polygon" << v.length() << "ends";
            v.append(Obstacle(poly, w));
        }
    }
    return v;
}

// getPath...

void Field::load(QString path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Невозможно открыть XML-конфиг";
        return;
    }
    QXmlStreamReader xml(&file);
    while (!xml.atEnd() && !xml.hasError())
    {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartElement)
        {
            if (xml.name() == QString("polygons"))
                this->obstacles = getObstacles(&xml);
            if (xml.name() == QString("path"))
                continue; //todo
        }
    }
}

void Field::save(QString path) {

}

unsigned Field::count() {
    return this->obstacles.length();
}

void Field::draw(QPainter* painter) {
    QPen p(Qt::red);
    p.setWidth(2);
    painter->setPen(p);
    painter->setBrush(Qt::green);

    for (Obstacle& obst : this->obstacles) {
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

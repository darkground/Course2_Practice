#include "field.h"
#include "utils.h"

Field::Field(unsigned w, unsigned h) {
    qDebug() << "Field::init" << w << h;
    this->width = w;
    this->height = h;
    regenMesh();
}

Field::~Field() {
    if (drawPoly != 0) delete drawPoly;
}

void Field::draw(QPainter* painter) {
    if (dGrid) {
        QPen p(dGridOutline ? outlineGrid : QColor(0, 0, 0, 0));
        painter->setPen(p);
        for (auto it = mesh.keyValueBegin(); it != mesh.keyValueEnd(); ++it) {
            QPoint startPoint = it->second.realCoord;
            QColor sqColor = mix(easyObstacle, hardObstacle, it->second.walkness);
            painter->setBrush(sqColor);
            painter->drawRect(startPoint.x(), startPoint.y(), cellSize, cellSize);
        }
    }

    if (!dNoObstacles) {
        QPen p(outlineObstacle, polyWidth);
        painter->setPen(p);
        painter->setFont(QFont("Times", 16));
        for (Obstacle& obst : obstacles) {
            QColor polyColor = mix(easyObstacle, hardObstacle, obst.walkness);
            painter->setBrush(polyColor);
            painter->drawPolygon(obst.poly);
            QPoint center = polygonCentroid(obst.poly);
            if (obst.poly.boundingRect().contains(center)) {
                QRect rect(center - QPoint(30, 30), QSize(60, 60));
                int w = obst.walkness * 100;
                painter->drawText(rect, Qt::AlignCenter, QString::number(w) + QString("%"));
            }
        }
    }

    if (!dNoPath) {
        QPen p(path);
        painter->setPen(p);
        for (int i = 1; i < way.length(); i++) {
            MeshPoint& prev = way[i-1];
            MeshPoint& next = way[i];
            painter->drawLine(prev.realCoord, next.realCoord);
        }
    }

    QPen p(outlineObstacle, pointWidth);
    painter->setPen(p);

    if (start.has_value()) {
        painter->setBrush(fillStart);
        painter->drawEllipse(*start, 6, 6);
    }

    if (end.has_value()) {
        painter->setBrush(fillEnd);
        painter->drawEllipse(*end, 6, 6);
    }
}

// Map -- Функции карты

//!
//! Загрузить карту из XML-файла
//!
//! \param path Путь до XML-файла
//! \return 0 в случае успеха
//! \return -1 если файл не найден
//! \return -2 если файл имеет неверную структуру
//! \return -3 если файл имеет неверные значения
//!
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

//!
//! Поменять размеры карты
//!
//! \param width Ширина
//! \param height Высота
//!
void Field::resizeMap(unsigned width, unsigned height) {
    obstacles.clear();
    this->width = width;
    this->height = height;
    qDebug() << "Field::size" << "Set to" << width << height;
    regenMesh();
}

//!
//! Проверка принадлежности точки полю
//!
//! \param point Точка
//! \return Принадлежность
//!
bool Field::inMap(const QPoint& point) {
    return point.x() >= 0 && point.y() >= 0 && point.x() < (int)width && point.y() < (int)height;
}

//!
//! Получение непроходимости в точке поля
//!
//! \param point Точка
//! \return Фактор непроходимости в этой точке поля
//!
double Field::getFactorMap(const QPoint& point) {
    for (Obstacle& obst : obstacles) {
        if (obst.poly.containsPoint(point, Qt::FillRule::OddEvenFill)) {
            return obst.walkness;
        }
    }
    return 0.;
}

// Mesh -- Сетка

//!
//! \brief Генерация сетки.
//! Генерирует сетку, ширина ячейки которой равна `Field::cellSize`.
//! Необходимый этап перед запуском нахождения кратчайшего пути
//!
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

//!
//! Ближайшая точка на сетке.
//! Получить ближайшую точку на сетке используя произвольную точку
//!
//! \param point Точка
//! \return Указатель на ближайшую точку или nullptr если не найдено
//!
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

//!
//! Получить точку сетки.
//! Получить точку на сетке используя произвольную точку
//!
//! \param point Точка
//! \return Указатель на точку сетки или nullptr если не найдено
//!
MeshPoint* Field::getMesh(const QPoint& point) {
    if (mesh.contains(point)) return &mesh[point];
    return 0;
}

// Points

//!
//! Установить старт
//!
//! \param point Точка старта
//!
void Field::setStart(QPoint point) {
    qInfo() << "Field::start" << point;
    start.emplace(point);
}

//!
//! Установить финиш
//!
//! \param point Точка финиша
//!
void Field::setEnd(QPoint point) {
    qInfo() << "Field::end" << point;
    end.emplace(point);
}

//!
//! Получить точку старта
//!
//! \return Точка старта или nullopt если она не задана
//!
Waypoint Field::getStart() {
    return start;
}

//!
//! Получить точку финиша
//!
//! \return Точка финиша или nullopt если она не задана
//!
Waypoint Field::getEnd() {
    return end;
}

// Polygon Editing -- Редактирование полигонов

//!
//! Получить препятствие по точке
//! Возвращает препятствие под точкой.
//! Так как препятствия не могут пересекаться, то на карте каждая точка может либо лежать в
//! только одном препятствии, либо не лежать вообще
//!
//! \param point Точка
//! \return Препятствие или nullptr если не найдено
//!
Obstacle* Field::getObstacle(const QPoint& point) {
    for (Obstacle& obst : obstacles) {
        if (obst.poly.containsPoint(point, Qt::FillRule::OddEvenFill)) {
            return &obst;
        }
    }
    return 0;
}

QVector<Obstacle>& Field::getObstacles() {
    return obstacles;
}

//!
//! Удалить препятствие по точке
//!
//! \param point Точка
//! \return Успех или нет
//!
bool Field::removeObstacle(const QPoint& point) {
    Obstacle* obst = getObstacle(point);
    if (obst == 0) return false;
    return removeObstacle(*obst);
}

//!
//! Удалить препятствие
//! Удаляемое препятствие должно присутствовать в списке препятствий поля
//!
//! \param obst Препятствие.
//! \return Успех или нет
//!
bool Field::removeObstacle(const Obstacle& obst) {
    qInfo() << "Field::remObst" << obst.walkness;
    if (obstacles.removeOne(obst)) {
        regenMesh();
        return true;
    }
    return false;
}


//!
//! Добавить точку в препятствие
//! Добавляемая точка должна лежать в пределах полигона препятствия
//!
//! \param point Точка
//!
bool Field::addToObstacle(const QPoint& point) {
    Obstacle* obst = getObstacle(point);
    if (obst == 0) return false;
    return addToObstacle(*obst, point);
}

//!
//! Добавить точку в препятствие
//! Добавляет точку в препятствие, разделяя ближайшее к этой точке ребро на две части,
//! соединённых данной точкой
//!
//! \param obst Препятствие
//! \param point Точка
//!
bool Field::addToObstacle(Obstacle& obst, const QPoint& point) {
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
    return true;
}

//!
//! Удалить точку из препятствия
//! Удаляемая точка должна лежать в пределах полигона препятствия
//!
//! \param point Точка
//! \return Успех или нет
//!
bool Field::removeFromObstacle(const QPoint& point) {
    Obstacle* obst = getObstacle(point);
    if (obst == 0) return false;
    return removeFromObstacle(*obst, point);
}

//!
//! Удалить точку из препятствия
//! Удаляет точку, которая лежит ближе всего к точке, данной пользователем
//!
//! \param obst Препятствие
//! \param point Точка
//! \return Успех или нет
//!
bool Field::removeFromObstacle(Obstacle& obst, const QPoint& point) {
    int idx = -1;
    float closest = width * height;

    for (int i = 0; i < obst.poly.size(); i++) {
        QPoint& pt = obst.poly[i];
        float dst = euclideanDistance(pt, point);
        if (dst < closest && dst <= 6) {// Field::pointGrabRadius) {
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

//!
//! Найти путь
//! Ищет кратчайший путь по сгенерированной раннее сетке с помощью алгоритма A*.
//! Если путь найден, то он сохранён в way.
//!
//! \return >0 если путь найден
//! \return ==0 если не получилось проложить путь от старта до финиша
//! \return -1 если старт/финиш не задан
//!
float Field::findPath() {
    if (!start.has_value() || !end.has_value()) return -1;
    MeshPoint* mstart = nearestMesh(*start);
    MeshPoint* mend = nearestMesh(*end);
    if (mstart == 0 || mend == 0) return -1;

    way.clear();
    float shortest = aStarPath(mstart, mend, way);
    qInfo() << "Field::find" << shortest << "/" << way.length();

    way = smoothv1Path(way);
    std::reverse(way.begin(), way.end());
    way = splicePath(way);
    way = smoothv1Path(way);
    return shortest;
}

//!
//! Под-функция A* для обработки соседних клеток в сетке
//! Если текущая позиция + смещение выходит за рамки карты, то функция завершается.
//! Если соседняя позиция имеет непроходимость 1.0, то препятствие считается стеной и его необходимо обойти
//!
//! \param queue Приоритетная очередь
//! \param origins Hash-карта показывающая, откуда проложен путь
//! \param costs Hash-карта показывающая, какая минимальная стоимость нужна для достижения этой клетки в сетке
//! \param current Указатель на текущую позицию в сетке
//! \param finish Цель (финиш)
//! \param offset Смещение по координатам
//!
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
    float new_cost = costs[current->meshCoord] + vectorLength(offset) + neighbor->walkness;
    if (!costs.contains(neighbor->meshCoord) || new_cost < costs[neighbor->meshCoord]) {
        costs[neighbor->meshCoord] = new_cost;
        origins[neighbor->meshCoord] = current->meshCoord;
        queue.put(neighbor, new_cost + simpleDistance(neighbor->meshCoord, finish->meshCoord));
    }
}

//!
//! Алгоритм поиска пути A*
//!
//! \param start Начальная точка
//! \param finish Конечная точка
//! \param way Вектор для сохранения пути
//! \return Длина пути, если путь найден
//! \return 0, если путь не найден
//!
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
        // Diagonals
        aStarN(queue, origins, costs, current, finish, QPoint(1, 1));
        aStarN(queue, origins, costs, current, finish, QPoint(1, -1));
        aStarN(queue, origins, costs, current, finish, QPoint(-1, 1));
        aStarN(queue, origins, costs, current, finish, QPoint(-1, -1));
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

//!
//! Получить количество препятствий на карте
//!
//! \return Количество препятствий
//!
unsigned Field::polyCount() {
    return obstacles.length();
}

//!
//! Сглаживание пути
//! Сглаживание пути быстрым методом линейного прохода
//!
//! \param Путь, который необходимо сгладить.
//!
QVector<MeshPoint> Field::smoothv1Path(const QVector<MeshPoint>& vec) {
    if (vec.length() < 1) return vec;
    QVector <MeshPoint> finalVec;
    int curr = 0;

    for (int i = 1; i < vec.length(); ++i) {
        QLine line(vec[curr].realCoord, vec[i].realCoord);
        for (int j = 0; j < obstacles.length(); ++j) {
            if (lineIntersectsPolygon(line, obstacles[j].poly)) {
                line.setP2(vec[i-1].realCoord);
                finalVec.append(vec[curr]);
                finalVec.append(vec[i-1]);
                curr = i-1;
                break;
            }
        }
    }
    finalVec.append(vec[curr]);
    finalVec.append(vec[vec.length() - 1]);
    return finalVec;
}

//!
//! Сглаживание пути
//! Сглаживание пути итеративным подходом упрощения
//!
//! \param Путь, который необходимо сгладить.
//!
QVector<MeshPoint> Field::smoothv2Path(const QVector<MeshPoint>& vec, int maxSteps) {
    if (vec.length() < 2) return vec;
    QVector <MeshPoint> reduced(vec);
    QVector <MeshPoint> ignorance;

    for (int it = 0; it < maxSteps && reduced.length() > 2; it++) {
        QVector<MeshPoint> next;
        next.append(reduced[0]);
        int i = 1;
        while (i < reduced.length()-1) {
            if (ignorance.contains(reduced[i])) {
                next.append(reduced[i]);
                i += 1;
                continue;
            }
            QLine line(reduced[i-1].realCoord, reduced[i+1].realCoord);
            bool inter = false;
            for (Obstacle& obst : obstacles) {
                if (lineIntersectsPolygon(line, obst.poly)) {
                    inter = true;
                    break;
                }
            }

            if (inter) {
                next.append(reduced[i]);
                ignorance.append(reduced[i]);
                i += 1;
            } else {
                next.append(reduced[i+1]);
                i += 2;
            }
        }
        next.append(reduced[reduced.length()-1]);
        reduced = next;
    }

    return reduced;
}

//!
//! Разделение пути
//! Разделить линии путя на точки с заданным интервалом
//!
//! \param Путь, который необходимо сгладить.
//!
QVector<MeshPoint> Field::splicePath(const QVector<MeshPoint>& vec, int interval) {
    QVector<MeshPoint> result;
    for (int i = 1; i < vec.length(); i++) {
        QLine formed(vec[i-1].realCoord, vec[i].realCoord);
        double dist = euclideanDistance(formed.p1(), formed.p2());
        double dx = formed.dx() / dist;
        double dy = formed.dy() / dist;
        for (int j = 0; j < dist; j += interval) {
            QPoint init = formed.p1();
            QPoint rp = QPoint(init.x() + dx * j, init.y() + dy * j);
            MeshPoint mp = MeshPoint(QPoint(-1, -1), rp, getFactorMap(rp));
            result.append(mp);
        }
        result.append(vec[i]);
    }
    return result;
}

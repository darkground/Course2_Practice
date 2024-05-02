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

// Polygon Drawing -- Функции рисования полигонов

//!
//! \brief Начать рисование полигона
//! Подготовить поле для рисования полигона
//!
void Field::startDraw() {
    qInfo() << "Field::polyDraw" << "Start";
    drawFlag = true;
    drawPoly = new QPolygon();
}

//!
//! Добавить точку в рисуемый полигон
//!
//! \param p Точка для добавления в полигон
//! \return Успех добавления
//!
//! Функция должна быть вызвана после startDraw() и до stopDraw(),
//! иначе эффекта не будет.
//!
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

//!
//! \brief Убрать последнюю нарисованную точку
//!
//! Ничего не произойдёт если рисуемый полигон не имеет точек.
//! Функция должна быть вызвана после startDraw() и до stopDraw(),
//! иначе эффекта не будет.
//!
void Field::undoDraw() {
    if (!drawFlag) return;
    if (!drawPoly->empty()) {
        qInfo() << "Field::polyDraw" << "Undo";
        drawPoly->removeLast();
    }
}

//!
//! Получить рисуемый полигон
//! \return Рисуемый полигон или nullptr если рисование не начато
//!
QPolygon* Field::getDraw() {
    return drawPoly;
}

//!
//! Подтвердить рисование полигона
//! Подтвердить рисование полигона, создав препятствие с данной непроходимостью.
//! Функция должна быть вызвана после startDraw() и до stopDraw(),
//! иначе эффекта не будет.
//! Эта функция автоматически останавливает рисовку, вызывая stopDraw().
//!
//! \param w Непроходимость полигона
//!
void Field::endDraw(double w) {
    if (!drawFlag) return;
    qInfo() << "Field::polyDraw" << "End, W =" << w;
    obstacles.append(Obstacle(*drawPoly, w));
    regenMesh();
    stopDraw();
}

//!
//! \brief Выйти из режима рисовки
//! Остановить рисовку и очистить все ресурсы.
//!
void Field::stopDraw() {
    qInfo() << "Field::polyDraw" << "Stop";
    drawFlag = false;
    delete drawPoly;
    drawPoly = 0;
}

// Polygon Editing -- Редактирование полигонов

//!
//! \brief Начать режим перемещения точки полигона
//! Подготовить поле к перемещению точки полигона пользователем.
//!
void Field::startDrag() {
    qInfo() << "Field::polyDrag" << "Start";
    dragFlag = true;
}

//!
//! Начать захват точки полигона
//! Начать захват точки полигона от точки, данной пользователем.
//! Если нашлись точки в радиусе Field::pointGrabRadius от данной точки,
//! то для захвата выбирается ближайшая из них.
//! Функция должна быть вызвана после startDrag() и до stopDrag(),
//! иначе эффекта не будет.
//!
//! \param from Точка поля
//!
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

//!
//! Передвинуть захваченную точку
//! Эта функция передвигает раннее захваченную точку в координаты, данные пользователем.
//! Если точка не захвачена, функция вернёт false.
//! Если точка находится вне карты, функция вернёт false.
//! Функция должна быть вызвана после startDrag() и до stopDrag(),
//! иначе эффекта не будет.
//!
//! \param where Куда передвинуть
//! \return Успех или нет
//!
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

//!
//! \brief Снять захват точки
//!
void Field::endDrag() {
    dragPoly = 0;
    dragPoint = 0;
    qInfo() << "Field::polyDrag" << "End";
}

//!
//! \brief Выйти из режима перемещения точки
//! Остановить перемещение и очистить ресурсы.
//! Данная функция автоматически вызывает регенерацию сетки, т.к. произошли изменения в структуре препятствий
//!
void Field::stopDrag() {
    dragFlag = false;
    regenMesh();
    qInfo() << "Field::polyDrag" << "Stop";
}

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
void Field::addPointObstacle(const QPoint& point) {
    Obstacle* obst = getObstacle(point);
    if (obst == 0) return;
    addPointObstacle(*obst, point);
}

//!
//! Добавить точку в препятствие
//! Добавляет точку в препятствие, разделяя ближайшее к этой точке ребро на две части,
//! соединённых данной точкой
//!
//! \param obst Препятствие
//! \param point Точка
//!
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

//!
//! Удалить точку из препятствия
//! Удаляемая точка должна лежать в пределах полигона препятствия
//!
//! \param point Точка
//! \return Успех или нет
//!
bool Field::removePointObstacle(const QPoint& point) {
    Obstacle* obst = getObstacle(point);
    if (obst == 0) return false;
    return removePointObstacle(*obst, point);
}

//!
//! Удалить точку из препятствия
//! Удаляет точку, которая лежит ближе всего к точке, данной пользователем
//!
//! \param obst Препятствие
//! \param point Точка
//! \return Успех или нет
//!
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
    smoothifyPath(way);
    qInfo() << "Field::find" << shortest << "/" << way.length();
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
    float new_cost = costs[current->meshCoord] + 1 + neighbor->walkness;
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
//! Данная функция изменяет массив, данный на вход.
//!
//! \param Ссылка на путь, который необходимо сгладить.
//!
void Field::smoothifyPath(QVector<MeshPoint>& vec) {
    QVector <QLine> lines;
    QVector <MeshPoint> finalVec;
    int curr = 0;
    // QVector <MeshPoint>::Iterator iter = vec.begin();

    for(int i = 1; i < vec.length(); ++i) {
        QLine line(vec[curr].realCoord, vec[i].realCoord);
        // lines.append(line);
        for(int j = 0; j < obstacles.length(); ++j) {
            bool test = lineIntersectsPolygon(line, obstacles[j].poly);
            qDebug() << line << obstacles[j].poly << test;
            if(test) {
                line.setP2(vec[i-1].realCoord);
                finalVec.append(vec[curr]);
                finalVec.append(vec[i-1]);
                curr = i-1;
                lines.append(line);
                break;
            }
        }
    }
    QLine line(vec[curr].realCoord, vec[vec.length() - 1].realCoord);
    lines.append(line);
    finalVec.append(vec[curr]);
    finalVec.append(vec[vec.length() - 1]);
    vec = finalVec;
}

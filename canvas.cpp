#include "canvas.h"
#include "utils.h"

Canvas::Canvas(QWidget* parent) : QWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_Hover);
}

Canvas::~Canvas() {
    delete field;
}

void Canvas::setAction(CanvasAction a) {
    switch (action) {
        case POLYGON_CREATE:
            endDraw();
            break;
        case POLYGON_DELETE:
            field->findPath();
            break;
        case START:
        case END:
            field->findPath();
        default:
            break;
    }
    switch (a) {
        case POLYGON_CREATE:
            startDraw();
            break;
        default:
            break;
    }
    update();
    action = a;
}

Field* Canvas::getField() {
    return field;
}

void Canvas::showEvent(QShowEvent*) {
    if (field == 0) {
        QSize canvasSize = minimumSize();
        resizeMap(canvasSize);
    }
}

void Canvas::paintEvent(QPaintEvent*) {
    QPainter painter;

    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QSize canvasSize = size();

    // Round box
    painter.setPen(QPen(Qt::black));
    painter.drawRect(0, 0, canvasSize.width(), canvasSize.height());

    // Drawing map
    field->draw(&painter);

    if (action == POLYGON_EDIT) {
        QPen p(Field::outlineDraw, Field::polyWidth);
        painter.setBrush(QColor(0, 0, 0, 0));
        for (Obstacle& o : field->getObstacles()) {
            for (QPoint& point : o.poly) {
                p.setColor(&point == drag ? Field::lastPointDraw : Field::pointDraw);
                painter.setPen(p);
                painter.drawEllipse(point, 6, 6);
            }
        }
    }

    if (action == POLYGON_CREATE) {
        QPen p(Field::outlineDraw, Field::polyWidth);
        painter.setPen(p);
        painter.setBrush(Field::easyObstacle);
        painter.drawPolygon(*draw);

        painter.setBrush(QColor(0, 0, 0, 0));
        for (QPoint& point : *draw) {
            p.setColor(point == draw->last() ? Field::lastPointDraw : Field::pointDraw);
            painter.setPen(p);
            painter.drawEllipse(point, 6, 6);
        }
    }

    QPen p(QColor(0, 0, 0, 150), Field::polyWidth);
    painter.setPen(p);
    painter.setFont(QFont("Consolas", 10));

    switch (action) {
        case WALKNESS:
            painter.drawText(QPoint(4, canvasSize.height() - 18), QString("Проверка проходимости"));
            painter.drawText(QPoint(4, canvasSize.height() - 6), QString("[ЛКМ] Получить проходимость"));
            break;
        case POLYGON_CREATE:
            painter.drawText(QPoint(4, canvasSize.height() - 18), QString("Создание препятствий"));
            painter.drawText(QPoint(4, canvasSize.height() - 6), QString("[ЛКМ] Добавить точку, [Shift+ЛКМ] Убрать точку, [ПКМ] Завершить, [Shift+ПКМ] Отмена"));
            break;
        case POLYGON_DELETE:
            painter.drawText(QPoint(4, canvasSize.height() - 18), QString("Удаление препятствий"));
            painter.drawText(QPoint(4, canvasSize.height() - 6), QString("[ЛКМ] Удалить препятствие, [ПКМ] Завершить"));
            break;
        case POLYGON_EDIT:
            painter.drawText(QPoint(4, canvasSize.height() - 18), QString("Редактирование препятствий"));
            painter.drawText(QPoint(4, canvasSize.height() - 6), QString("[ЛКМ] Двигать точку, [Shift+ЛКМ] Добавить точку, [ПКМ] Завершить, [Shift+ПКМ] Удалить точку"));
            break;
        case START:
            painter.drawText(QPoint(4, canvasSize.height() - 18), QString("Установка начальной точки"));
            painter.drawText(QPoint(4, canvasSize.height() - 6), QString("[ЛКМ] Установить начальную точку, [ПКМ] Завершить"));
            break;
        case END:
            painter.drawText(QPoint(4, canvasSize.height() - 18), QString("Установка финиша"));
            painter.drawText(QPoint(4, canvasSize.height() - 6), QString("[ЛКМ] Установить финиш, [ПКМ] Завершить"));
            break;
    }

    painter.end();
}

bool Canvas::event(QEvent* e) {
    switch(e->type()) {
        case QEvent::HoverLeave: {
            emit coordMoved(QPoint(-1, -1));
            return true;
        }
        case QEvent::HoverMove: {
            QHoverEvent* event = (QHoverEvent*)e;
            emit coordMoved(event->position().toPoint());
            return true;
        }
        default:
            break;
    }
    return QWidget::event(e);
}

void Canvas::mousePressEvent(QMouseEvent* event) {
    QPoint pos = event->pos();
    switch (action) {
        case POLYGON_EDIT: {
            if (event->button() == Qt::LeftButton) {
                if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) startDrag(pos);
                else field->addToObstacle(pos);
            }
            update();
            break;
        }
        default:
            break;
    }
}

void Canvas::mouseMoveEvent(QMouseEvent* event) {
    QPoint pos = event->pos();
    switch (action) {
        case POLYGON_EDIT: {
            if (moveDrag(pos)) {
                emit statusUpdated(QString("Изменение препятствия: точка перемещена в [%1, %2]").arg(pos.x()).arg(pos.y()));
                update();
            }
            break;
        }
        default:
            break;
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent* event) {
    QPoint pos = event->pos();
    switch (action) {
        case WALKNESS: {
            if (event->button() == Qt::LeftButton) {
                int w = field->getFactorMap(pos) * 100;
                emit statusUpdated(QString("Непроходимость в [%1, %2] = %3").arg(pos.x()).arg(pos.y()).arg(w) + QString("%"));
            }
            break;
        }
        case START: {
            if (event->button() == Qt::LeftButton) {
                field->setStart(pos);
                emit statusUpdated(QString("Старт: установлен в [%1, %2]").arg(pos.x()).arg(pos.y()));
            } else {
                setAction(WALKNESS);
                emit statusUpdated(QString("Старт: установка завершена"));
            }
            update();
            break;
        }
        case END: {
            if (event->button() == Qt::LeftButton) {
                field->setEnd(pos);
                emit statusUpdated(QString("Финиш: установлен в [%1, %2]").arg(pos.x()).arg(pos.y()));
            } else {
                setAction(WALKNESS);
                emit statusUpdated(QString("Финиш: установка завершена"));
            }
            update();
            break;
        }
        case POLYGON_CREATE: {
            if (event->button() == Qt::LeftButton) {
                if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
                    if (doDraw(pos)) {
                        emit statusUpdated(QString("Создание препятствия: %1 точек").arg(draw->count()));
                    } else {
                        emit statusUpdated(QString("Создание препятствия: препятствия не должны пересекаться"));
                    }
                } else {
                    undoDraw();
                    emit statusUpdated(QString("Создание препятствия: %1 точек").arg(draw->count()));
                }
            } else if (event->button() == Qt::RightButton) {
                if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
                    bool ok;
                    double w = QInputDialog::getDouble(this, "Непроходимость", "Введите непроходиость:", 0.5, 0., 1., 2, &ok, Qt::WindowFlags(), 0.01);
                    if (ok) {
                        confirmDraw(w);
                        field->findPath();
                        setAction(WALKNESS);
                        emit objectsUpdated(field->polyCount());
                        emit statusUpdated(QString("Создание препятствия: завершено"));
                    }
                } else {
                    setAction(WALKNESS);
                    emit statusUpdated(QString("Создание препятствия: действие отменено"));
                }
            }
            update();
            break;
        }
        case POLYGON_DELETE: {
            if (event->button() == Qt::LeftButton) {
                if (field->removeObstacle(pos)) {
                    emit objectsUpdated(field->polyCount());
                    emit statusUpdated(QString("Удаление препятствия: удалено"));
                } else emit statusUpdated(QString("Удаление препятствия: препятствие не найдено"));
            } else if (event->button() == Qt::RightButton) {
                setAction(WALKNESS);
                emit statusUpdated(QString("Удаление препятствия: завершено"));
            }
            update();
            break;
        }
        case POLYGON_EDIT: {
            if (event->button() == Qt::LeftButton) endDrag();
            else {
                if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
                    endDrag();
                    field->regenMesh();
                    field->findPath();
                    setAction(WALKNESS);
                    emit statusUpdated(QString("Изменение препятствия: завершено"));
                } else {
                    if (field->removeFromObstacle(pos)) {
                        emit objectsUpdated(field->polyCount());
                        emit statusUpdated(QString("Изменение препятствия: точка удалена"));
                    } else emit statusUpdated(QString("Изменение препятствия: точка не найдена"));
                }
            }
            update();
            break;
        }
        default:
            break;
    }
}

void Canvas::loadMap(const QString& path) {
    int code = field->loadMap(path);
    switch (code) {
    case -1:
        emit statusUpdated(QString("Загрузка карты: XML-файл не найден"));
        break;
    case -2:
        emit statusUpdated(QString("Загрузка карты: Структура XML-файла нарушена"));
        break;
    case -3:
        emit statusUpdated(QString("Загрузка карты: XML-файл хранит недопустимые значения"));
        break;
    default:
        setMinimumSize(field->size());
        emit sizeChanged(field->size());
        emit statusUpdated(QString("Загрузка карты: XML-файл успешно загружен"));
        emit objectsUpdated(field->polyCount());
        update();
        break;
    }
}

void Canvas::saveMap(const QString& path) {
    int code = field->saveMap(path);
    switch (code) {
    case -1:
        emit statusUpdated(QString("Сохранение карты: неудачно"));
        break;
    default:
        emit statusUpdated(QString("Сохранение карты: сохранено"));
        break;
    }
}

void Canvas::resizeMap(QSize size) {
    field = new Field(size.width(), size.height());
    setMinimumSize(size);
    update();
    emit sizeChanged(size);
}

// Polygon Editing -- Редактирование полигонов

void Canvas::startDrag(QPoint point) {
    endDrag();
    float closest = Field::pointGrabRadius;
    for (Obstacle& obst : field->getObstacles()) {
        for (QPoint& vertex : obst.poly) {
            float dst = euclideanDistance(point, vertex);
            if (dst < closest && dst <= Field::pointGrabRadius) {
                attach = &obst.poly;
                drag = &vertex;
                closest = dst;
            }
        }
    }
}

bool Canvas::moveDrag(QPoint point) {
    if (attach == 0 || !field->inMap(point)) return false;
    QPolygon it = *attach;
    QPoint old = *drag;

    *drag = point;

    for (Obstacle& obst : field->getObstacles()) {
            if (obst.poly != it && obst.poly.intersects(it)) {
                *drag = old;
                break;
            }
    }

    return true;
}

void Canvas::endDrag() {
    attach = 0;
    drag = 0;
}

// Polygon Drawing -- Функции рисования полигонов

void Canvas::startDraw() {
    draw = new QPolygon();
}

bool Canvas::doDraw(QPoint point) {
    if (draw == 0) return false;
    if (field->getFactorMap(point) != 0.) return false;
    QPolygon poly(*draw);
    poly << point;
    for (Obstacle& o : field->getObstacles()) {
            if (o.poly.intersects(poly)) return false;
    }
    (*draw) << point;
    return true;
}

void Canvas::undoDraw() {
    if (draw == 0) return;
    if (!draw->empty()) {
            draw->removeLast();
    }
}

void Canvas::confirmDraw(float w) {
    if (draw == 0) return;
    field->getObstacles().append(Obstacle(*draw, w));
    field->regenMesh();
}

void Canvas::endDraw() {
    delete draw;
    draw = 0;
}

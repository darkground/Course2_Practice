#include "canvas.h"
#include "qapplication.h"

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
            field->stopDraw();
            break;
        case POLYGON_EDIT:
            field->stopDrag();
            break;
        case POLYGON_DELETE:
            field->regenMesh();
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
            field->startDraw();
            break;
        case POLYGON_EDIT:
            field->startDrag();
            break;
        default:
            break;
    }
    update();
    action = a;
}

CanvasAction Canvas::getAction() {
    return action;
}

Field* Canvas::getField() {
    return field;
}

void Canvas::showEvent(QShowEvent*) {
    if (field == 0) {
        qDebug() << "Canvas::show";
        QSize canvasSize = minimumSize();
        field = new Field(canvasSize.width(), canvasSize.height());
    }
}

void Canvas::paintEvent(QPaintEvent*) {
    QPainter p;
    p.begin(this);
    p.setRenderHint(QPainter::Antialiasing);

    QSize canvasSize = size();

    // Round box
    p.setPen(QPen(Qt::black));
    p.drawRect(0, 0, canvasSize.width(), canvasSize.height());

    // Drawing map
    field->draw(&p);

    p.end();
}

bool Canvas::event(QEvent* e) {
    switch(e->type()) {
        case QEvent::HoverLeave: {
        emit coordMoved(QPoint(0, 0));
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
    switch (action) {
        case POLYGON_EDIT: {
            if (event->button() == Qt::LeftButton) {
                QPoint pos = event->pos();
                if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
                    field->beginDrag(pos);
                } else field->addToObstacle(pos);
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
            if (field->moveDrag(pos)) {
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
                field->findPath();
                emit statusUpdated(QString("Старт: установка завершена"));
                action = WALKNESS;
            }
            update();
            break;
        }
        case END: {
            if (event->button() == Qt::LeftButton) {
                field->setEnd(pos);
                emit statusUpdated(QString("Финиш: установлен в [%1, %2]").arg(pos.x()).arg(pos.y()));
            } else {
                field->findPath();
                emit statusUpdated(QString("Финиш: установка завершена"));
                action = WALKNESS;
            }
            update();
            break;
        }
        case POLYGON_CREATE: {
            if (event->button() == Qt::LeftButton) {
                if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
                    if (field->doDraw(pos)) {
                        emit statusUpdated(QString("Создание препятствия: %1 точек").arg(field->getDraw()->count()));
                    } else {
                        emit statusUpdated(QString("Создание препятствия: препятствия не должны пересекаться"));
                    }
                } else {
                    field->undoDraw();
                    emit statusUpdated(QString("Создание препятствия: %1 точек").arg(field->getDraw()->count()));
                }
            } else if (event->button() == Qt::RightButton) {
                if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
                    bool ok;
                    double w = QInputDialog::getDouble(this, "Непроходимость", "Введите непроходиость:", 0.5, 0., 1., 2, &ok, Qt::WindowFlags(), 0.01);
                    if (ok) {
                        field->endDraw(w);
                        field->findPath();
                        emit objectsUpdated(field->polyCount());
                        emit statusUpdated(QString("Создание препятствия: завершено"));
                        action = WALKNESS;
                    }
                } else {
                    field->stopDraw();
                    emit statusUpdated(QString("Создание препятствия: действие отменено"));
                    action = WALKNESS;
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
                field->regenMesh();
                field->findPath();
                emit statusUpdated(QString("Удаление препятствия: завершено"));
                action = WALKNESS;
            }
            update();
            break;
        }
        case POLYGON_EDIT: {
            if (event->button() == Qt::LeftButton) field->finishDrag();
            else {
                if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
                    if (field->removeFromObstacle(pos)) {
                        emit objectsUpdated(field->polyCount());
                        emit statusUpdated(QString("Изменение препятствия: точка удалена"));
                    } else emit statusUpdated(QString("Изменение препятствия: точка не найдена"));
                } else {
                    field->endDrag();
                    field->findPath();
                    emit statusUpdated(QString("Изменение препятствия: завершено"));
                    action = WALKNESS;
                }
            }
            update();
            break;
        }
        default:
            break;
    }
}

void Canvas::loadMap(QString path) {
    QSize canvasSize = size();
    field->resizeMap(canvasSize.width(), canvasSize.height());
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
            emit statusUpdated(QString("Загрузка карты: XML-файл успешно загружен"));
            emit objectsUpdated(field->polyCount());
            update();
            break;
    }
}

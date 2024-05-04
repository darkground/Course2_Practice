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

void Canvas::showEvent(QShowEvent*)
{
    if (field == 0) {
        qDebug() << "Canvas::show";
        QSize canvasSize = minimumSize();
        field = new Field(canvasSize.width(), canvasSize.height());
    }
}

void Canvas::paintEvent(QPaintEvent*)
{
    QPainter p;
    p.begin(this);
    p.setRenderHint(QPainter::Antialiasing);

    QSize canvasSize = size();

    // Round box
    p.setPen(QPen(Qt::black));
    p.drawRect(0, 0, canvasSize.width() - 1, canvasSize.height() - 1);

    // Drawing map
    field->draw(&p);

    p.end();
}

bool Canvas::event(QEvent* e)
{
    switch(e->type()) {
        case QEvent::HoverLeave: {
            emit coords(QPoint(0, 0));
            return true;
        }
        case QEvent::HoverMove: {
            QHoverEvent* event = (QHoverEvent*)e;
            emit coords(event->position().toPoint());
            return true;
        }
        default:
            break;
    }
    return QWidget::event(e);
}

void Canvas::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
        case Qt::Key_D:
            debugKey = true;
            break;
    }
    update();
}

void Canvas::keyReleaseEvent(QKeyEvent* event)
{
    switch (event->key()) {
        case Qt::Key_D:
            debugKey = false;
            break;
        case Qt::Key_G: // [G]rid
            if (!debugKey) break;
            if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
                field->dGrid = !field->dGrid;
                emit status(QString("Отладка: переключение сетки"));
            } else {
                field->dGridOutline = !field->dGridOutline;
                emit status(QString("Отладка: переключение границ сетки"));
            }
            update();
            break;
        case Qt::Key_O: // [O]bstacles
            if (!debugKey) break;
            field->dNoObstacles = !field->dNoObstacles;
            update();
            emit status(QString("Отладка: переключение видимости препятствий"));
            break;
        case Qt::Key_P: // [P]ath
            if (!debugKey) break;
            field->dNoPath = !field->dNoPath;
            update();
            emit status(QString("Отладка: переключение видимости путей"));
            break;
        case Qt::Key_Up: // Raise grid size
            if (!debugKey) break;
            field->cellSize *= 2;
            if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
                field->regenMesh();
                update();
                emit status(QString("Отладка: увеличить разрешение сетки до %1").arg(field->cellSize));
            } else {
                emit status(QString("Отладка: увеличить разрешение сетки до %1 (без регенерации)").arg(field->cellSize));
            }
            break;
        case Qt::Key_Down: // Lower grid size (min 2)
            if (!debugKey) break;
            if (field->cellSize != 2) field->cellSize /= 2;
            if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
                field->regenMesh();
                update();
                emit status(QString("Отладка: снизить разрешение сетки до %1").arg(field->cellSize));
            } else {
                emit status(QString("Отладка: снизить разрешение сетки до %1 (без регенерации)").arg(field->cellSize));
            }
            break;
        case Qt::Key_M: // [M]esh regen
            if (!debugKey) break;
            field->regenMesh();
            update();
            emit status(QString("Отладка: переключение видимости путей"));
            break;
    }
    update();
}

void Canvas::mousePressEvent(QMouseEvent* event) {
    switch (action) {
        case POLYGON_EDIT: {
            if (event->button() == Qt::LeftButton) {
                QPoint pos = event->pos();
                if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
                    field->beginDrag(pos);
                } else field->addPointObstacle(pos);
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
                emit status(QString("Изменение препятствия: точка перемещена в [%1, %2]").arg(pos.x()).arg(pos.y()));
                update();
            }
            break;
        }
        default:
            break;
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent* event)
{
    QPoint pos = event->pos();
    switch (action) {
        case WALKNESS: {
            if (event->button() == Qt::LeftButton) {
                int w = field->getFactorMap(pos) * 100;
                emit status(QString("Непроходимость в [%1, %2] = %3").arg(pos.x()).arg(pos.y()).arg(w) + QString("%"));
            }
            break;
        }
        case START: {
            if (event->button() == Qt::LeftButton) {
                field->setStart(pos);
                emit status(QString("Старт: установлен в [%1, %2]").arg(pos.x()).arg(pos.y()));
            } else {
                field->findPath();
                emit status(QString("Старт: установка завершена"));
                action = WALKNESS;
            }
            update();
            break;
        }
        case END: {
            if (event->button() == Qt::LeftButton) {
                field->setEnd(pos);
                emit status(QString("Финиш: установлен в [%1, %2]").arg(pos.x()).arg(pos.y()));
            } else {
                field->findPath();
                emit status(QString("Финиш: установка завершена"));
                action = WALKNESS;
            }
            update();
            break;
        }
        case POLYGON_CREATE: {
            if (event->button() == Qt::LeftButton) {
                if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
                    if (field->doDraw(pos)) {
                        emit status(QString("Создание препятствия: %1 точек").arg(field->getDraw()->count()));
                    } else {
                        emit status(QString("Создание препятствия: препятствия не должны пересекаться"));
                    }
                } else {
                    field->undoDraw();
                    emit status(QString("Создание препятствия: %1 точек").arg(field->getDraw()->count()));
                }
            } else if (event->button() == Qt::RightButton) {
                if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
                    bool ok;
                    double w = QInputDialog::getDouble(this, "Непроходимость", "Введите непроходиость:", 0.5, 0., 1., 2, &ok, Qt::WindowFlags(), 0.01);
                    if (ok) {
                        field->endDraw(w);
                        field->findPath();
                        emit objects(field->polyCount());
                        emit status(QString("Создание препятствия: завершено"));
                        action = WALKNESS;
                    }
                } else {
                    field->stopDraw();
                    emit status(QString("Создание препятствия: действие отменено"));
                    action = WALKNESS;
                }
            }
            update();
            break;
        }
        case POLYGON_DELETE: {
            if (event->button() == Qt::LeftButton) {
                if (field->removeObstacle(pos)) {
                    emit objects(field->polyCount());
                    emit status(QString("Удаление препятствия: удалено"));
                } else emit status(QString("Удаление препятствия: препятствие не найдено"));
            } else if (event->button() == Qt::RightButton) {
                field->regenMesh();
                field->findPath();
                emit status(QString("Удаление препятствия: действие отменено"));
                action = WALKNESS;
            }
            update();
            break;
        }
        case POLYGON_EDIT: {
            if (event->button() == Qt::LeftButton) field->finishDrag();
            else {
                if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
                    if (field->removePointObstacle(pos)) {
                        emit objects(field->polyCount());
                        emit status(QString("Изменение препятствия: точка удалена"));
                    } else emit status(QString("Изменение препятствия: точка не найдена"));
                } else {
                    field->endDrag();
                    field->findPath();
                    emit status(QString("Изменение препятствия: завершено"));
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

void Canvas::load(QString path)
{
    QSize canvasSize = size();
    field->resizeMap(canvasSize.width(), canvasSize.height());
    int code = field->loadMap(path);
    switch (code) {
        case -1:
            emit status(QString("Загрузка карты: XML-файл не найден"));
            break;
        case -2:
            emit status(QString("Загрузка карты: Структура XML-файла нарушена"));
            break;
        case -3:
            emit status(QString("Загрузка карты: XML-файл хранит недопустимые значения"));
            break;
        default:
            emit status(QString("Загрузка карты: XML-файл успешно загружен"));
            emit objects(field->polyCount());
            update();
            break;
    }
}

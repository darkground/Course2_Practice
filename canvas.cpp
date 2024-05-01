#include "canvas.h"
#include "qapplication.h"

Canvas::Canvas(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_Hover);
}

Canvas::~Canvas() {
    delete this->field;
}

void Canvas::setAction(CanvasAction a) {
    this->action = a;
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
    this->field->draw(&p);

    p.end();
}

bool Canvas::event(QEvent* e)
{
    switch(e->type())
    {
    //case QEvent::HoverEnter: {
    //    QHoverEvent* event = (QHoverEvent*)e;
    //    qDebug() << Q_FUNC_INFO << event->type();
    //    return true;
    //}
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

void Canvas::showEvent(QShowEvent* e)
{
    QSize canvasSize = minimumSize();
    this->field = new Field(canvasSize.width(), canvasSize.height());
}

void Canvas::mouseReleaseEvent(QMouseEvent* event)
{
    switch (this->action) {
    case WALKNESS: {
        if (event->button() == Qt::LeftButton) {
            QPoint pos = event->pos();
            int w = this->field->getFactor(event->pos()) * 100;
            emit status(QString("Непроходимость в [%1, %2] = %3").arg(pos.x()).arg(pos.y()).arg(w) + QString("%"));
        }
        break;
    }
    case START: {
        if (event->button() == Qt::LeftButton) {
            QPoint pos = event->pos();
            this->field->setStart(pos);
            emit status(QString("Старт: установлен в [%1, %2]").arg(pos.x()).arg(pos.y()));
        } else {
            emit status(QString("Старт: установка отменена"));
        }
        this->field->find();
        this->action = WALKNESS;
        update();
        break;
    }
    case END: {
        if (event->button() == Qt::LeftButton) {
            QPoint pos = event->pos();
            this->field->setEnd(pos);
            emit status(QString("Финиш: установлена в [%1, %2]").arg(pos.x()).arg(pos.y()));
        } else {
            emit status(QString("Финиш: действие отменено"));
        }
        this->field->find();
        this->action = WALKNESS;
        update();
        break;
    }
    case POLYGON_CREATE: {
        if (event->button() == Qt::LeftButton) {
            if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
                if (this->field->addDraw(event->pos())) {
                    int c = this->field->getDraw().count();
                    emit status(QString("%1 точек: [ЛКМ] Добавить точку, [Shift+ЛКМ] Убрать точку, [ПКМ] Завершить, [Shift+ПКМ] Отмена").arg(c));
                } else {
                    emit status(QString("Создание препятствия: препятствия не должны пересекаться"));
                }
            } else {
                this->field->removeDraw();
                int c = this->field->getDraw().count();
                emit status(QString("%1 точек: [ЛКМ] Добавить точку, [Shift+ЛКМ] Убрать точку, [ПКМ] Завершить, [Shift+ПКМ] Отмена").arg(c));
            }
        } else if (event->button() == Qt::RightButton) {
            if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
                bool ok;
                float w = (float)QInputDialog::getDouble(this, "Непроходимость", "Введите непроходиость:", 0.5, 0., 1., 2, &ok, Qt::WindowFlags(), 0.01);
                if (ok) {
                    this->field->finishDraw(w);
                    emit objects(this->field->count());
                    emit status(QString("Создание препятствия: завершено"));
                    this->action = WALKNESS;
                }
            } else {
                this->field->cancelDraw();
                emit status(QString("Создание препятствия: действие отменено"));
                this->action = WALKNESS;
            }
        }
        update();
        break;
    }
    case POLYGON_DELETE: {
        if (event->button() == Qt::LeftButton) {
            if (this->field->removeAt(event->pos())) {
                emit objects(this->field->count());
                emit status(QString("Удаление препятствия: удалено"));
                this->action = WALKNESS;
            } else {
                emit status(QString("Удаление препятствия: препятствие не найдено"));
            }
        } else if (event->button() == Qt::RightButton) {
            emit status(QString("Удаление препятствия: действие отменено"));
            this->action = WALKNESS;
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
    this->field->resizeMap(canvasSize.width(), canvasSize.height());
    int code = this->field->loadMap(path);
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
        emit objects(this->field->count());
        update();
        break;
    }
}

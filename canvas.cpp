#include "canvas.h"

Canvas::Canvas(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_Hover);
    QSize canvasSize = size();
    this->field = new Field(canvasSize.width(), canvasSize.height());
}

Canvas::~Canvas() {
    delete this->field;
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

    // Drawing polygons
    this->field->draw(&p);

    p.end();
}

bool Canvas::event(QEvent* e)
{
    switch(e->type())
    {
    case QEvent::HoverEnter: {
        QHoverEvent* event = (QHoverEvent*)e;
        qDebug() << Q_FUNC_INFO << event->type();
        return true;
    }
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

void Canvas::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint pos = event->pos();
        int w = this->field->getFactor(event->pos()) * 100;
        emit status(QString("Непроходимость в [%1, %2] = %3").arg(pos.x()).arg(pos.y()).arg(w) + QString("%"));
    }
}

void Canvas::load(QString path)
{
    QSize canvasSize = size();
    this->field->resize(canvasSize.width(), canvasSize.height());
    int code = this->field->load(path);
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

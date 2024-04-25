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
        qDebug() << "w =" << this->field->getFactor(event->pos());
    }
}

void Canvas::load(QString path)
{
    QSize canvasSize = size();
    this->field->resize(canvasSize.width(), canvasSize.height());
    int code = this->field->load(path);
    switch (code) {
    case -1:
        emit status(QString("xml не найден"));
        break;
    case -2:
        emit status(QString("нарушена структура xml-файла"));
        break;
    case -3:
        emit status(QString("xml-файл хранит недопустимые значения"));
        break;
    default:
        emit status(QString("xml успешно загружен"));
        emit objects(this->field->count());
        update();
        break;
    }
}

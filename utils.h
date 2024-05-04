#ifndef UTILS_H
#define UTILS_H

#include <QColor>
#include <QLine>
#include <QPolygon>
#include <QtMath>

//!
//! Смешивание с помощью фактора между двумя цветами
//!
//! \param c1 Первый цвет (factor = 0)
//! \param c2 Второй цвет (factor = 1)
//! \param factor Фактор
//! \return Смешанный цвет
//!
QColor mix(const QColor& c1, const QColor& c2, double factor) {
    return QColor(
        c1.red() * (1 - factor) + c2.red() * factor,
        c1.green() * (1 - factor) + c2.green() * factor,
        c1.blue() * (1 - factor) + c2.blue() * factor,
        255
    );
}

//!
//! Евклидова дистанция между двумя точками
//! Дистанция вычисляется как корень суммы квадратов разностей координат точек
//!
//! \param p1 Точка 1
//! \param p2 Точка 2
//! \return Дистанция
//!
double euclideanDistance(const QPoint& p1, const QPoint& p2) {
    return qSqrt(qPow(p1.x() - p2.x(), 2) + qPow(p1.y() - p2.y(), 2));
}

//!
//! Простая дистанция между двумя точками
//! Дистанция вычисляется как сумма модулей разностей координат точек
//!
//! \param p1 Точка 1
//! \param p2 Точка 2
//! \return Дистанция
//!
double simpleDistance(const QPoint& p1, const QPoint& p2) {
    return qAbs(p1.x() - p2.x()) + qAbs(p1.y() - p2.y());
}

double vectorLength(const QPoint& p1) {
    return qSqrt(qPow(p1.x(), 2) + qPow(p1.y(), 2));
}

//!
//! Проецировать точку на линию
//!
//! \param l Линия
//! \param p Точка
//! \return Проецированная точка
//!
QPoint nearestPointOnLine(const QLine& l, const QPoint& p){
    double APx = p.x() - l.x1();
    double APy = p.y() - l.y1();
    double ABx = l.x2() - l.x1();
    double ABy = l.y2() - l.y1();
    double magAB2 = ABx*ABx + ABy*ABy;
    double ABdotAP = ABx*APx + ABy*APy;
    double t = ABdotAP / magAB2;

    QPoint newPoint;

    if (t < 0) {
        newPoint = l.p1();
    } else if (t > 1){
        newPoint = l.p2();
    } else {
        newPoint.setX(l.x1() + ABx*t);
        newPoint.setY(l.y1() + ABy*t);
    }

    return newPoint;
}

//!
//! Проверка на пересечение линии и полигона
//!
//! \param line Линия
//! \param polygon Полигон
//! \return Успех или нет
//!
bool lineIntersectsPolygon(const QLine& line, const QPolygon& polygon) {
    QPolygon t;
    t << line.p1() << line.p2();
    return polygon.intersects(t);
}

#endif // UTILS_H

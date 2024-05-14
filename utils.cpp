//!
//! Функции, которые используются в проекте,
//! но которые не получилось присвоить какому-то конкретному классу
//!

#include "utils.h"

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

//!
//! Найти длину вектора
//!
//! \param p1 Вектор
//! \return Длина
//!
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

//!
//! Найти центроид полигона
//!
//! \param poly Полигон
//! \return Сентроид
//!
QPoint polygonCentroid(const QPolygon& poly) {
    QPoint centroid = {0, 0};
    double signedArea = 0.0;
    int x0 = 0.0;
    int y0 = 0.0;
    int x1 = 0.0;
    int y1 = 0.0;
    int a = 0.0;

    for (int i = 0; i < poly.size() - 1; ++i) {
        x0 = poly[i].x();
        y0 = poly[i].y();
        x1 = poly[i+1].x();
        y1 = poly[i+1].y();
        a = x0 * y1 - x1 * y0;
        signedArea += a;
        centroid.rx() += (x0 + x1) * a;
        centroid.ry() += (y0 + y1) * a;
    }

    x0 = poly[poly.size()-1].x();
    y0 = poly[poly.size()-1].y();
    x1 = poly[0].x();
    y1 = poly[0].y();
    a = x0 * y1 - x1 * y0;
    signedArea += a;
    centroid.rx() += (x0 + x1)*a;
    centroid.ry() += (y0 + y1)*a;

    signedArea *= 0.5;
    centroid.rx() /= (6.0 * signedArea);
    centroid.ry() /= (6.0 * signedArea);

    return centroid;
}

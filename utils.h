#ifndef UTILS_H
#define UTILS_H

#include <QColor>
#include <QLine>

QColor mix(const QColor& c1, const QColor& c2, float factor) {
    return QColor(
        c1.red() * (1 - factor) + c2.red() * factor,
        c1.green() * (1 - factor) + c2.green() * factor,
        c1.blue() * (1 - factor) + c2.blue() * factor,
        255
    );
}

QPoint nearestPoint(const QLine& liner, const QPoint& pointr){
    double APx = pointr.x() - liner.x1();
    double APy = pointr.y() - liner.y1();
    double ABx = liner.x2() - liner.x1();
    double ABy = liner.y2() - liner.y1();
    double magAB2 = ABx*ABx + ABy*ABy;
    double ABdotAP = ABx*APx + ABy*APy;
    double t = ABdotAP / magAB2;

    QPoint newPoint;

    if (t < 0) {
        newPoint = liner.p1();
    }else if (t > 1){
        newPoint = liner.p2();
    }else{
        newPoint.setX(liner.x1() + ABx*t);
        newPoint.setY(liner.y1() + ABy*t);
    }

    return newPoint;
}

#endif // UTILS_H

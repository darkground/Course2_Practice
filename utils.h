#ifndef UTILS_H
#define UTILS_H

#include <QColor>
#include <QLine>
#include <QPolygon>
#include <QtMath>

QColor mix(const QColor& c1, const QColor& c2, double factor);
double euclideanDistance(const QPoint& p1, const QPoint& p2);
double simpleDistance(const QPoint& p1, const QPoint& p2);
double vectorLength(const QPoint& p1);
QPoint nearestPointOnLine(const QLine& l, const QPoint& p);
bool lineIntersectsPolygon(const QLine& line, const QPolygon& polygon);
QPoint polygonCentroid(const QPolygon& poly);

#endif // UTILS_H

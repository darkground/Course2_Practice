#ifndef CANVAS_H
#define CANVAS_H

#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QEvent>
#include <QHoverEvent>
#include <QLabel>
#include <QInputDialog>
#include "field.h"
#include "mainwindow.h"

enum CanvasAction {
    WALKNESS = 0,
    START = 1,
    END = 2,
    POLYGON_CREATE = 3,
    POLYGON_DELETE = 4,
    POLYGON_EDIT = 5
};

class Canvas : public QWidget
{
    Q_OBJECT

public:
    Canvas(QWidget* parent = 0);
    ~Canvas();

    void loadMap(QString path);
    //void save(QString path);

    void setAction(CanvasAction a);

    Field* getField();

signals:
    void coordMoved(QPoint p);
    void statusUpdated(QString s);
    void objectsUpdated(unsigned c);

protected:
    Field* field = 0;
    CanvasAction action = WALKNESS;

    QPolygon* draw = 0;
    QPolygon* attach = 0;
    QPoint* drag = 0;

    bool event(QEvent* e);
    void showEvent(QShowEvent* event);
    void paintEvent(QPaintEvent*);
    void mouseReleaseEvent(QMouseEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);

    void startDrag(QPoint point);
    bool moveDrag(QPoint point);
    void endDrag();

    void startDraw();
    bool doDraw(QPoint point);
    void undoDraw();
    void confirmDraw(float w);
    void endDraw();

private:
    Ui::MainWindow* ui;
};

#endif // CANVAS_H

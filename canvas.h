#ifndef CANVAS_H
#define CANVAS_H

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

    void load(QString path);
    //void save(QString path);
    void setAction(CanvasAction a);

signals:
    void coords(QPoint p);
    void status(QString s);
    void objects(unsigned c);

protected:
    Field* field = 0;
    CanvasAction action = WALKNESS;

    void showEvent(QShowEvent* event);
    void paintEvent(QPaintEvent*);
    bool event(QEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);

private:
    Ui::MainWindow* ui;
};

#endif // CANVAS_H

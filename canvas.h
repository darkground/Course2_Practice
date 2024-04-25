#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include <QPainter>
#include <QEvent>
#include <QHoverEvent>
#include <QLabel>
#include "field.h"
#include "mainwindow.h"

class Canvas : public QWidget
{
    Q_OBJECT

public:
    Canvas(QWidget* parent = 0);
    ~Canvas();

    void load(QString path);
    //void save(QString path);

signals:
    void coords(QPoint p);
    void status(QString s);
    void objects(unsigned c);

protected:
    Field* field = 0;

    void paintEvent(QPaintEvent*);
    bool event(QEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);

private:
    Ui::MainWindow* ui;
};

#endif // CANVAS_H

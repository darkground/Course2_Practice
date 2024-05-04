#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->widgetGraph, &Canvas::coordMoved, this, &MainWindow::coords);
    connect(ui->widgetGraph, &Canvas::objectsUpdated, this, &MainWindow::objects);
    connect(ui->widgetGraph, &Canvas::statusUpdated, this, &MainWindow::status);

    connect(ui->btnStart, &QPushButton::clicked, this, &MainWindow::setStart);
    connect(ui->btnFinish, &QPushButton::clicked, this, &MainWindow::setEnd);
    connect(ui->btnAdd, &QPushButton::clicked, this, &MainWindow::createPoly);
    connect(ui->btnDelete, &QPushButton::clicked, this, &MainWindow::deletePoly);
    connect(ui->btnEdit, &QPushButton::clicked, this, &MainWindow::editPoly);
    
    connect(ui->btnLoad, &QPushButton::clicked, this, &MainWindow::load);
    //connect(ui->btnSave, &QPushButton::clicked, this, &MainWindow::save);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
    case Qt::Key_D:
        debugKey = true;
        break;
    }
    update();
}

void MainWindow::keyReleaseEvent(QKeyEvent* event) {
    Field* field = ui->widgetGraph->getField();
    switch (event->key()) {
        case Qt::Key_D:
            debugKey = false;
            break;
        case Qt::Key_G: // [G]rid
            if (!debugKey) break;
            if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
                field->dGrid = !field->dGrid;
                status(QString("Отладка: переключение сетки"));
            } else {
                field->dGridOutline = !field->dGridOutline;
                status(QString("Отладка: переключение границ сетки"));
            }
            update();
            break;
        case Qt::Key_O: // [O]bstacles
            if (!debugKey) break;
            field->dNoObstacles = !field->dNoObstacles;
            update();
            status(QString("Отладка: переключение видимости препятствий"));
            break;
        case Qt::Key_P: // [P]ath
            if (!debugKey) break;
            field->dNoPath = !field->dNoPath;
            update();
            status(QString("Отладка: переключение видимости путей"));
            break;
        case Qt::Key_Up: // Raise grid size
            if (!debugKey) break;
            field->cellSize *= 2;
            if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
                field->regenMesh();
                update();
                status(QString("Отладка: увеличить разрешение сетки до %1").arg(field->cellSize));
            } else {
                status(QString("Отладка: увеличить разрешение сетки до %1 (без регенерации)").arg(field->cellSize));
            }
            break;
        case Qt::Key_Down: // Lower grid size (min 2)
            if (!debugKey) break;
            if (field->cellSize != 2) field->cellSize /= 2;
            if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
                field->regenMesh();
                update();
                status(QString("Отладка: снизить разрешение сетки до %1").arg(field->cellSize));
            } else {
                status(QString("Отладка: снизить разрешение сетки до %1 (без регенерации)").arg(field->cellSize));
            }
            break;
        case Qt::Key_M: // [M]esh regen
            if (!debugKey) break;
            field->regenMesh();
            update();
            status(QString("Отладка: переключение видимости путей"));
            break;
    }
    ui->widgetGraph->update();
}

void MainWindow::coords(QPoint p) {
    QString s = QString("Координаты: [%1, %2]").arg(p.x()).arg(p.y());
    ui->labelCoords->setText(s);
}

void MainWindow::objects(unsigned c) {
    QString s = QString("%1 объектов").arg(c);
    ui->labelObjects->setText(s);
}

void MainWindow::status(QString s) {
    ui->labelStatus->setText(s);
}

void MainWindow::load() {
    ui->widgetGraph->loadMap("data.xml");
    ui->widgetGraph->update();
}

void MainWindow::setStart() {
    ui->widgetGraph->setAction(CanvasAction::START);
    ui->labelStatus->setText(QString("[ЛКМ] Установить начальную точку, [ПКМ] Завершить"));
}

void MainWindow::setEnd() {
    ui->widgetGraph->setAction(CanvasAction::END);
    ui->labelStatus->setText(QString("[ЛКМ] Установить финиш, [ПКМ] Завершить"));
}

void MainWindow::createPoly() {
    ui->widgetGraph->setAction(CanvasAction::POLYGON_CREATE);
    ui->labelStatus->setText(QString("[ЛКМ] Добавить точку, [Shift+ЛКМ] Убрать точку, [ПКМ] Завершить, [Shift+ПКМ] Отмена"));
}

void MainWindow::deletePoly() {
    ui->widgetGraph->setAction(CanvasAction::POLYGON_DELETE);
    ui->labelStatus->setText(QString("[ЛКМ] Удалить препятствие, [ПКМ] Завершить"));
}

void MainWindow::editPoly() {
    ui->widgetGraph->setAction(CanvasAction::POLYGON_EDIT);
    ui->labelStatus->setText(QString("[ЛКМ] Двигать точку, [Shift+ЛКМ] Добавить точку, [ПКМ] Удалить точку, [Shift+ПКМ] Завершить"));
}

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    act.setColor(QPalette::Window, Qt::blue);

    connect(ui->widgetGraph, &Canvas::coordMoved, this, &MainWindow::coordMoved);
    connect(ui->widgetGraph, &Canvas::objectsUpdated, this, &MainWindow::objectsUpdated);
    connect(ui->widgetGraph, &Canvas::statusUpdated, this, &MainWindow::statusUpdated);

    connect(ui->btnStart, &QPushButton::clicked, this, &MainWindow::actionStart);
    connect(ui->btnFinish, &QPushButton::clicked, this, &MainWindow::actionEnd);
    connect(ui->btnAdd, &QPushButton::clicked, this, &MainWindow::actionCreate);
    connect(ui->btnDelete, &QPushButton::clicked, this, &MainWindow::actionDelete);
    connect(ui->btnEdit, &QPushButton::clicked, this, &MainWindow::actionEdit);
    
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
                statusUpdated(QString("Отладка: переключение сетки"));
            } else {
                field->dGridOutline = !field->dGridOutline;
                statusUpdated(QString("Отладка: переключение границ сетки"));
            }
            update();
            break;
        case Qt::Key_O: // [O]bstacles
            if (!debugKey) break;
            field->dNoObstacles = !field->dNoObstacles;
            update();
            statusUpdated(QString("Отладка: переключение видимости препятствий"));
            break;
        case Qt::Key_P: // [P]ath
            if (!debugKey) break;
            field->dNoPath = !field->dNoPath;
            update();
            statusUpdated(QString("Отладка: переключение видимости путей"));
            break;
        case Qt::Key_Up: // Raise grid size
            if (!debugKey) break;
            field->cellSize *= 2;
            if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
                field->regenMesh();
                update();
                statusUpdated(QString("Отладка: увеличить разрешение сетки до %1").arg(field->cellSize));
            } else {
                statusUpdated(QString("Отладка: увеличить разрешение сетки до %1 (без регенерации)").arg(field->cellSize));
            }
            break;
        case Qt::Key_Down: // Lower grid size (min 2)
            if (!debugKey) break;
            if (field->cellSize != 2) field->cellSize /= 2;
            if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
                field->regenMesh();
                update();
                statusUpdated(QString("Отладка: снизить разрешение сетки до %1").arg(field->cellSize));
            } else {
                statusUpdated(QString("Отладка: снизить разрешение сетки до %1 (без регенерации)").arg(field->cellSize));
            }
            break;
        case Qt::Key_M: // [M]esh regen
            if (!debugKey) break;
            field->regenMesh();
            update();
            statusUpdated(QString("Отладка: переключение видимости путей"));
            break;
        case Qt::Key_1:
            actionWalk();
            break;
        case Qt::Key_2:
            actionCreate();
            break;
        case Qt::Key_3:
            actionDelete();
            break;
        case Qt::Key_4:
            actionEdit();
            break;
        case Qt::Key_5:
            actionStart();
            break;
        case Qt::Key_6:
            actionEnd();
            break;
    }
    ui->widgetGraph->update();
}

void MainWindow::coordMoved(QPoint p) {
    QString s = QString("Координаты: [%1, %2]").arg(p.x()).arg(p.y());
    ui->labelCoords->setText(s);
}

void MainWindow::objectsUpdated(unsigned c) {
    QString s = QString("%1 объектов").arg(c);
    ui->labelObjects->setText(s);
}

void MainWindow::statusUpdated(QString s) {
    ui->labelStatus->setText(s);
}

void MainWindow::load() {
    ui->widgetGraph->loadMap("data.xml");
    ui->widgetGraph->update();
}

void MainWindow::actionWalk() {
    ui->widgetGraph->setAction(CanvasAction::WALKNESS);
}

void MainWindow::actionStart() {
    ui->widgetGraph->setAction(CanvasAction::START);
}

void MainWindow::actionEnd() {
    ui->widgetGraph->setAction(CanvasAction::END);
}

void MainWindow::actionCreate() {
    ui->widgetGraph->setAction(CanvasAction::POLYGON_CREATE);
}

void MainWindow::actionDelete() {
    ui->widgetGraph->setAction(CanvasAction::POLYGON_DELETE);
}

void MainWindow::actionEdit() {
    ui->widgetGraph->setAction(CanvasAction::POLYGON_EDIT);
}

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->widgetGraph, &Canvas::coords, this, &MainWindow::coords);
    connect(ui->widgetGraph, &Canvas::objects, this, &MainWindow::objects);
    connect(ui->widgetGraph, &Canvas::status, this, &MainWindow::status);

    connect(ui->btnLoad, &QPushButton::clicked, this, &MainWindow::load);
    connect(ui->btnStart, &QPushButton::clicked, this, &MainWindow::setStart);
    connect(ui->btnFinish, &QPushButton::clicked, this, &MainWindow::setEnd);
    connect(ui->btnAdd, &QPushButton::clicked, this, &MainWindow::createPoly);
    connect(ui->btnDelete, &QPushButton::clicked, this, &MainWindow::deletePoly);
    //connect(ui->btnSave, &QPushButton::clicked, this, &MainWindow::save);
}

MainWindow::~MainWindow()
{
    delete ui;
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
    ui->widgetGraph->load("data.xml");
    ui->widgetGraph->update();
}

void MainWindow::setStart() {
    ui->widgetGraph->setAction(CanvasAction::START);
    ui->labelStatus->setText(QString("[ЛКМ] Установить начальную точку, [ПКМ] Отмена"));
}

void MainWindow::setEnd() {
    ui->widgetGraph->setAction(CanvasAction::END);
    ui->labelStatus->setText(QString("[ЛКМ] Установить финиш, [ПКМ] Отмена"));
}

void MainWindow::createPoly() {
    ui->widgetGraph->setAction(CanvasAction::POLYGON_CREATE);
    ui->labelStatus->setText(QString("[ЛКМ] Добавить точку, [Shift+ЛКМ] Убрать точку, [ПКМ] Завершить, [Shift+ПКМ] Отмена"));
}

void MainWindow::deletePoly() {
    ui->widgetGraph->setAction(CanvasAction::POLYGON_DELETE);
    ui->labelStatus->setText(QString("[ЛКМ] Удалить препятствие, [ПКМ] Отмена"));
}

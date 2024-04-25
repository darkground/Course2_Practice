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

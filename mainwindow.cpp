//!
//! Класс главного окна/интерфейса.
//!

#include <QFileDialog>
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
    connect(ui->widgetGraph, &Canvas::sizeChanged, this, &MainWindow::sizeChanged);

    connect(ui->btnStart, &QPushButton::clicked, this, &MainWindow::actionStart);
    connect(ui->btnFinish, &QPushButton::clicked, this, &MainWindow::actionEnd);
    connect(ui->btnAdd, &QPushButton::clicked, this, &MainWindow::actionCreate);
    connect(ui->btnDelete, &QPushButton::clicked, this, &MainWindow::actionDelete);
    connect(ui->btnEdit, &QPushButton::clicked, this, &MainWindow::actionEdit);
    
    connect(ui->btnLoad, &QPushButton::clicked, this, &MainWindow::mapLoad);
    connect(ui->btnSave, &QPushButton::clicked, this, &MainWindow::mapSave);

    connect(ui->labelSize, &QPushButton::clicked, this, &MainWindow::resizeField);
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
        case Qt::Key_1: // [1] Walkness
            actionWalk();
            break;
        case Qt::Key_2: // [2] Creating
            actionCreate();
            break;
        case Qt::Key_3: // [3] Deleting
            actionDelete();
            break;
        case Qt::Key_4: // [4] Editing
            actionEdit();
            break;
        case Qt::Key_5: // [5] Start point
            actionStart();
            break;
        case Qt::Key_6: // [6] End point
            actionEnd();
            break;
    }
    ui->widgetGraph->update();
}

//!
//! Функция для обработки изменения координат поля.
//!
//! \param p Точка
//!
void MainWindow::coordMoved(QPoint p) {
    if (p == QPoint(-1, -1)) {
        QString s = QString("X, Y: [-, -]");
        ui->labelCoords->setText(s);
    } else {
        QString s = QString("X, Y: [%1, %2]").arg(p.x()).arg(p.y());
        ui->labelCoords->setText(s);
    }
}

//!
//! Функция для обработки изменения количества объектов
//!
//! \param c Количество объектов
//!
void MainWindow::objectsUpdated(unsigned c) {
    QString s = QString("%1 объектов").arg(c);
    ui->labelObjects->setText(s);
}

//!
//! Функция для обработки изменения статуса
//!
//! \param s Текст статуса
//!
void MainWindow::statusUpdated(QString s) {
    ui->labelStatus->setText(s);
}

//!
//! Функция для обработки изменения размеров карты
//!
//! \param sz Размеры
//!
void MainWindow::sizeChanged(QSize sz) {
    QString s = QString("%1 x %2").arg(sz.width()).arg(sz.height());
    ui->labelSize->setText(s);
}

//!
//! Функция загрузки карты. Вызывается кнопкой на форме
//!
void MainWindow::mapLoad() {
    QString fi = QFileDialog::getOpenFileName(this, "Загрузить карту", QString(), "XML files (*.xml)");
    if (!fi.isEmpty() && !fi.isNull()) {
        ui->widgetGraph->loadMap(fi);
        ui->widgetGraph->update();
    }
}

//!
//! Функция сохранения карты. Вызывается кнопкой на форме
//!
void MainWindow::mapSave() {
    QString fi = QFileDialog::getSaveFileName(this, "Сохранить карту", QString(), "XML files (*.xml)");
    if (!fi.isEmpty() && !fi.isNull()) {
        ui->widgetGraph->saveMap(fi);
    }
}

//!
//! Функция смены действия на проверку проходимости.
//! Вызывается когда любое другое действие отменено или клавишей [1]
//!
void MainWindow::actionWalk() {
    ui->widgetGraph->setAction(CanvasAction::WALKNESS);
}

//!
//! Функция смены действия на создание препятствия.
//! Вызывается кнопкой на форме или клавишей [2]
//!
void MainWindow::actionCreate() {
    ui->widgetGraph->setAction(CanvasAction::POLYGON_CREATE);
}

//!
//! Функция смены действия на удаление препятствия.
//! Вызывается кнопкой на форме или клавишей [3]
//!
void MainWindow::actionDelete() {
    ui->widgetGraph->setAction(CanvasAction::POLYGON_DELETE);
}

//!
//! Функция смены действия на изменения препятствия.
//! Вызывается кнопкой на форме или клавишей [4]
//!
void MainWindow::actionEdit() {
    ui->widgetGraph->setAction(CanvasAction::POLYGON_EDIT);
}

//!
//! Функция смены действия на установку старта.
//! Вызывается кнопкой на форме или клавишей [5]
//!
void MainWindow::actionStart() {
    ui->widgetGraph->setAction(CanvasAction::START);
}

//!
//! Функция смены действия на установку финиша.
//! Вызывается кнопкой на форме или клавишей [6]
//!
void MainWindow::actionEnd() {
    ui->widgetGraph->setAction(CanvasAction::END);
}

//!
//! Функция изменения размеров карты.
//! Вызывается кнопкой на форме справа снизу
//!
void MainWindow::resizeField() {
    bool ok;
    int w = QInputDialog::getInt(this, "Изменение размера карты", "Введите ширину:", 800, 100, 2000, 1, &ok, Qt::WindowFlags());
    if (!ok) return;
    int h = QInputDialog::getInt(this, "Изменение размера карты", "Введите высоту:", 500, 100, 2000, 1, &ok, Qt::WindowFlags());
    if (!ok) return;
    ui->widgetGraph->resizeMap(QSize(w, h));
}

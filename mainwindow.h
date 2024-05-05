#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void mapLoad();
    void mapSave();
    void actionWalk();
    void actionStart();
    void actionEnd();
    void actionCreate();
    void actionDelete();
    void actionEdit();
    void resizeField();

    void coordMoved(QPoint p);
    void objectsUpdated(unsigned c);
    void statusUpdated(QString s);
    void sizeChanged(QSize s);


private:
    Ui::MainWindow *ui;
    QPalette def;
    QPalette act;

    bool debugKey = false;

    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
};
#endif // MAINWINDOW_H

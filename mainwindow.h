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
    void load();
    //void save();
    void setStart();
    void setEnd();
    void createPoly();
    void deletePoly();
    void editPoly();

    void coords(QPoint p);
    void objects(unsigned c);
    void status(QString s);

private:
    Ui::MainWindow *ui;
    bool debugKey = false;

    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
};
#endif // MAINWINDOW_H

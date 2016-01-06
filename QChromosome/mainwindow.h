#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void openSimulation();

    void bb();
    void ab();
    void pb();
    void pf();
    void af();
    void ff();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

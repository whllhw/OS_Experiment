#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "pcb.h"
#include "sched.h"
#include <QMainWindow>
#include <QtSql>
#include <center_model.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
private slots:
    void add_task();
    void add_random_task();
    void update();

private:
    Ui::MainWindow* ui;
    scheduler schedu;
    QTimer* timer;
    MySqlQueryModel* back_model;
    QSqlTableModel* write_model;

    void add_toolBar();
    void init_model();
    void all_query();
};

#endif // MAINWINDOW_H

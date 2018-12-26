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

    void on_pushButton_clicked();

    void on_block_tableView_clicked(const QModelIndex& index);

    void on_pushButton_2_clicked();

    void on_spinBox_editingFinished();

    void on_spinBox_2_editingFinished();

    void on_pushButton_3_clicked();

private:
    Ui::MainWindow* ui;
    scheduler* schedu;
    //    sched_highpriority* schedu;
    QTimer* timer;
    MySqlQueryModel back_model[5];
    QSqlTableModel* write_model;
    unsigned long long counter = 0;
    bool is_current_highpriority = true;

    void add_toolBar();
    void init_model();
    void all_query();
    void hide_lcd();
};

#endif // MAINWINDOW_H

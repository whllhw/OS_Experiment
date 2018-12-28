#include "mainwindow.h"
#include "dialog.h"
#include "ui_mainwindow.h"
#include <QAction>
#include <QHeaderView>
#include <QIcon>
#include <QMessageBox>
#include <QTimer>
#include <QtSql>
#include <connection.h>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    add_toolBar();
    createConnection();
    init_model();
    schedu = new scheduler(write_model, ui->statusBar);
    is_current_highpriority = false;
    timer = new QTimer(this);
    connect(timer, timer->timeout, this, update);
    timer->start(1000);
}
void MainWindow::init_model()
{
    write_model = new QSqlTableModel();
    write_model->setTable("task");

    ui->back_tableView->setModel(&back_model[0]);
    ui->ready_tableView->setModel(&back_model[1]);
    ui->block_tableView->setModel(&back_model[2]);
    ui->finish_tableView->setModel(&back_model[3]);
    ui->run_tableView->setModel(&back_model[4]);
}

void MainWindow::add_toolBar()
{
    // 新建
    QToolBar* add_task_tool_bar = addToolBar(tr(""));
    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    QAction* newAct = new QAction(newIcon, tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new task"));
    connect(newAct, &QAction::triggered, this, &MainWindow::add_task);
    add_task_tool_bar->addAction(newAct);
    // 随机
    QAction* newAct_2 = new QAction(
        QIcon::fromTheme("document-new", QIcon(":/images/random.png")),
        tr("&Random"), this);
    newAct_2->setShortcuts(QKeySequence::Cut);
    newAct_2->setStatusTip(tr("random task"));
    connect(newAct_2, &QAction::triggered, this, &MainWindow::add_random_task);
    addToolBar(tr(""))->addAction(newAct_2);
    // 退出
    QAction* newAct_1 = new QAction(
        QIcon::fromTheme("document-new", QIcon(":/images/exit.png")),
        tr("&Exit"), this);
    newAct_1->setShortcuts(QKeySequence::Quit);
    newAct_1->setStatusTip(tr("Exit program"));
    connect(newAct_1, &QAction::triggered, this, &MainWindow::close);
    addToolBar(tr(""))->addAction(newAct_1);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::add_task()
{
    Dialog* dig = new Dialog(this);
    if (dig->exec()) {
        //        QMessageBox::information(this, tr("title"), tr("1"));
        task_struct* task = dig->get_task_struct();
        schedu->add_task(task);
    }
}

void MainWindow::add_random_task()
{

    const char* c = "0123456789abcdefghijklmnopqrstuvwxyzQWERTYUIOPASDFGHJKLZXCVBNM"; //可填充的字符
    int ulMsgLength = 6;
    char* MsgBuffer = new char[ulMsgLength]; //定义一个ulMsgLength长度的字符数组
    MsgBuffer[ulMsgLength - 1] = '\0';
    for (int i = 0; i < ulMsgLength - 1; i++) {
        int index = rand() % strlen(c);
        MsgBuffer[i] = c[index];
    }
    schedu->add_task(MsgBuffer, rand() % 10 + 1, rand() % 200 + 1);
}

void MainWindow::update()
{
    schedu->update();
    all_query();
    ui->lcdNumber_3->display(QString::number(counter++));
    ui->lcdNumber_4->display(QString::number(schedu->counter));
}

void MainWindow::all_query()
{
    // 后备队列
    back_model[0].setQuery("select pid as PID,task_name as 进程名,"
                           "priority as 优先级,remain_runtime as 剩余时间 from task "
                           "where state = '"
        + schedu->untils(TASK_ACCPECT) + "'");
    // 就绪队列
    back_model[1].setQuery("select pid as PID,task_name as 进程名,"
                           "priority as 优先级,remain_runtime as 剩余时间 from task "
                           "where state = '"
        + schedu->untils(TASK_RUNNING) + "'"
                                         " and pid <> "
        + QString::number(
              schedu->running_task == nullptr ? 0 : schedu->running_task->pid)
        + (is_current_highpriority ? " order by priority desc" : ""));
    // 阻塞队列
    back_model[2].setQuery("select pid as PID,task_name as 进程名,"
                           "priority as 优先级,remain_runtime as 剩余时间 from task "
                           "where state = '"
        + schedu->untils(TASK_STOPPED) + "'");
    // 设置完成队列信息
    back_model[3].setQuery("select pid as PID,task_name as 进程名,"
                           "priority as 优先级,remain_runtime as 剩余时间 from task "
                           "where state = 'TASK_FINISHED' or state = 'TASK_KILLED'");
    // 运行队列
    back_model[4].setQuery("select pid as PID,task_name as 进程名,"
                           "priority as 优先级,remain_runtime as 剩余时间 from task "
                           "where pid="
        + QString::number(schedu->running_task == nullptr ? 0 : schedu->running_task->pid));
    ui->back_tableView->resizeColumnsToContents();
    ui->run_tableView->resizeColumnsToContents();
    ui->finish_tableView->resizeColumnsToContents();
    ui->block_tableView->resizeColumnsToContents();
    ui->ready_tableView->resizeColumnsToContents();
}

void MainWindow::on_pushButton_clicked()
{
    schedu->block_current_task();
}

void MainWindow::on_block_tableView_clicked(const QModelIndex& index)
{
    QPoint pt;
    pt.setX(0);
    pt.setY(index.row());
    qDebug() << ui->block_tableView->indexAt(pt).data();
    schedu->unblock_task(ui->block_tableView->indexAt(pt).data().value<unsigned int>());
}

void MainWindow::on_pushButton_2_clicked()
{
    if (schedu->running_task == nullptr) {
        return;
    }
    schedu->kill_task(schedu->running_task->pid);
}

void MainWindow::on_spinBox_editingFinished()
{
    schedu->size_counter = ui->spinBox->value();
}

void MainWindow::on_spinBox_2_editingFinished()
{
    schedu->size_back_queue = ui->spinBox_2->value();
}

void MainWindow::on_pushButton_3_clicked()
{
    if (ui->radioButton->isChecked() && !is_current_highpriority) {
        is_current_highpriority = true;
        schedu = new sched_highpriority(*schedu);
        //        QMessageBox::information(this, "成功", "切换为优先权调度");
        ui->statusBar->showMessage("切换为优先权调度", 10);
    } else if (ui->radioButton_2->isChecked() && is_current_highpriority) {
        is_current_highpriority = false;
        schedu = new scheduler(*schedu);
        //        QMessageBox::information(this, "成功", "切换为时间片调度");
        ui->statusBar->showMessage("切换为时间片调度", 10);
    }
    hide_lcd();
}

void MainWindow::hide_lcd()
{
    if (is_current_highpriority) {
        ui->lcdNumber_4->hide();
        ui->label_3->hide();
    } else {
        ui->lcdNumber_4->show();
        ui->label_3->show();
    }
}

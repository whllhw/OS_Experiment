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
    schedu = new scheduler(write_model);
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
    all_query();
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
        QMessageBox::information(this, tr("title"), tr("1"));
    }
}

void MainWindow::add_random_task()
{

    const char* c = "0123456789abcdefghijklmnopqrstuvwxyz"; //可填充的字符
    int ulMsgLength = 5;
    char* MsgBuffer = new char[ulMsgLength]; //定义一个ulMsgLength长度的字符数组
    MsgBuffer[ulMsgLength - 1] = '\0';
    for (int i = 0; i < ulMsgLength - 1; i++) {
        int index = rand() % strlen(c);
        MsgBuffer[i] = c[index];
    }
    schedu->add_task(MsgBuffer, rand(), rand());
}

void MainWindow::update()
{
    schedu->update();
    all_query();
}

void MainWindow::all_query()
{
    //    for (int i = 0; i < 3; i++) {
    back_model[0].setQuery("select pid as PID,task_name as 进程名,"
                           "priority as 优先级,remain_runtime as 剩余时间 from task "
                           "where state = '"
        + schedu->untils(TASK_ACCPECT) + "'");
    //    }
    back_model[1].setQuery("select pid as PID,task_name as 进程名,"
                           "priority as 优先级,remain_runtime as 剩余时间 from task "
                           "where state = '"
        + schedu->untils(TASK_RUNNING) + "'");
    back_model[2].setQuery("select pid as PID,task_name as 进程名,"
                           "priority as 优先级,remain_runtime as 剩余时间 from task "
                           "where state = '"
        + schedu->untils(TASK_STOPPED) + "'");
    // 设置完成队列信息
    back_model[3].setQuery("select pid as PID,task_name as 进程名,"
                           "priority as 优先级,remain_runtime as 剩余时间 from task "
                           "where state = 'TASK_FINISHED' or state = 'TASK_KILLED'");
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

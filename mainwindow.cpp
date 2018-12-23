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
    timer = new QTimer(this);
    connect(timer, timer->timeout, this, update);
    timer->start(1000);
}
void MainWindow::init_model()
{
    write_model = new QSqlTableModel();
    write_model->setTable("task");

    back_model = new MySqlQueryModel();
    ui->back_tableView->setModel(back_model);
    all_query();
    ui->back_tableView->resizeColumnsToContents();
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
    MsgBuffer[ulMsgLength - 1] = 0;
    for (int i = 0; i < ulMsgLength; i++) {
        int index = rand() % strlen(c);
        MsgBuffer[i] = c[index];
    }
    schedu.add_task(MsgBuffer, rand(), rand());

    //    write_model->insertRecord(-1, )
}

void MainWindow::update()
{
    write_model->setFilter(" state = 'TASK_ACCPECT'");
    write_model->select();
    for (int i = 0; i < write_model->rowCount(); ++i) {
        QSqlRecord record = write_model->record(i);
        int remain_time = record.value(T_remain_runtime).toInt();
        remain_time--;
        record.setValue(T_remain_runtime, remain_time);
        write_model->setRecord(i, record);
    }
    write_model->submitAll();
    all_query();
}

void MainWindow::all_query()
{
    back_model->setQuery("select pid as PID,task_name as 进程名,priority as 优先级,remain_runtime as 剩余时间 from task "
                         "where state = 'TASK_ACCPECT'");
}

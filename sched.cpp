#include "sched.h"
#include "connection.h"
#include "pcb.h"
#include <QMutexLocker>
#include <QString>
#include <QtDebug>
#include <algorithm>
/**
 * 初始化各种队列
 * @param write_model 数据库连接
 * @param statusBar   状态栏
 */
scheduler::scheduler(QSqlTableModel* write_model, QStatusBar* statusBar)
{
    // 初始化根进程，用于双向链表
    root_task = new task_struct("root", 0, 0);
    root_task->state = TASK_UBERRUPTIBLE;
    this->write_model = write_model;
    this->statusBar = statusBar;
    ready_queue = new QQueue<task_struct*>();
    block_queue = new QList<task_struct*>();
    back_queue = new QList<task_struct*>();
    mutex = new QMutex();
}
/**
 * 添加任务到后备队列，设置进程状态为 TASK_ACCPECT
 * 1.添加进数据库
 * 2.添加进双向链表
 * @param task 任务
 * @return
 */
bool scheduler::add_task(task_struct* task)
{

    // 先放入后备队列中
    task->state = TASK_ACCPECT;
    QSqlQuery query;
    query.prepare("insert into task values"
                  "(:pid,:task_name,:state,:priority,"
                  ":total_runtime,:remain_runtime)");
    query.bindValue(":pid", task->pid);
    query.bindValue(":task_name", task->name);
    query.bindValue(":state", untils(TASK_ACCPECT));
    query.bindValue(":priority", task->priority);
    query.bindValue(":total_runtime", task->sum_exec_runtime);
    query.bindValue(":remain_runtime", task->sum_exec_runtime);
    query.exec();

    this->back_queue->append(task);
    // 放入双向链表
    return true;
}
/**
 * 添加一个进程并放入后备队列中
 */
bool scheduler::add_task(const char* task_name, unsigned long priority, unsigned long sum_exec_runtime)
{
    task_struct* task = new task_struct(task_name, priority, sum_exec_runtime);
    return add_task(task);
}
/**
 * 从后备队列中取出一个高优先级的放到就绪队列
 * 设置任务状态为TASK_RUNNING
 */
void scheduler::update_ready_queue()
{
    if (ready_queue->length() > size_back_queue) {
        //        qDebug() << "就绪队列数量大于" << size_back_queue;
        return;
    }
    if (back_queue->length() == 0) {
        //        qDebug() << "后备队列数量为0";
        return;
    }
    std::sort(back_queue->begin(), back_queue->end(),
        [](task_struct* s1, task_struct* s2) { return s1->priority > s2->priority; });
    task_struct* task = back_queue->at(0);
    back_queue->removeAt(0);
    model_set_state(task, TASK_RUNNING);
    QMutexLocker locker(mutex);
    ready_queue->enqueue(task);
}
/**
 * 从就绪队列取出一个进程运行
 * 发生调度情况如下：
 * 1.当前进程时间片用完。重新入队就绪
 * 2.当前进程阻塞。入队阻塞 TASK_STOPPED
 * 3.当前进程被杀死。标记为杀死 TASK_KILLED
 * 4.当前进程执行完毕。标记为结束 TASK_FINISHED
 */
void scheduler::scheduling()
{
    if (this->running_task != nullptr) {
        // 放入就绪队列末尾
        if (running_task->remain_exec_runtime <= 0) {
            model_set_state(running_task, TASK_FINISHED);
            running_task = nullptr;
        } else {
            if (running_task->state != TASK_RUNNING) {
                running_task = nullptr;
            } else {
                QMutexLocker locker(mutex);
                this->ready_queue->enqueue(this->running_task);
            }
        }
    } else {
        //        qDebug() << "当前无运行中的进程";
        statusBar->showMessage("当前无运行中的进程");
    }
    if (this->ready_queue->empty()) {
        //        qDebug() << "就绪队列为空";
        return;
    }
    task_struct* task = this->ready_queue->dequeue();
    this->running_task = task;
    counter = size_counter;
}

/**
 * 按时间片进行轮转，可能发生调度如下：
 * 1.当前进程完成
 * 2.时间片完
 * 3.当前进程状态不为运行（杀死、阻塞）
 */
void scheduler::update()
{
    update_ready_queue();
    if (running_task != nullptr) {
        if (model_count_remaintime() <= 0
            || --counter < 0
            || running_task->state != TASK_RUNNING) {
            scheduling();
        }
    } else {
        scheduling();
    }
}
/**
 * 减小当前进程的remaintime
 * @return 当前进程的remaintime
 */
int scheduler::model_count_remaintime()
{
    unsigned int pid = running_task->pid;
    write_model->setFilter("pid=" + QString::number(pid));
    write_model->select();
    if (write_model->rowCount() == 0) {
        qDebug() << __LINE__ << ",cannot find pid:" << pid;
        return 0;
    }
    QSqlRecord record = write_model->record(0);
    int remain_time = record.value(T_remain_runtime).toInt();
    remain_time--;
    record.setValue(T_remain_runtime, remain_time);
    write_model->setRecord(0, record);
    write_model->submitAll();
    running_task->remain_exec_runtime = remain_time;
    statusBar->showMessage("当前运行程序：" + QString(running_task->name));
    return remain_time;
}
/**
 * 设置进程的状态，同时设置model
 * @param task
 * @param state
 */
void scheduler::model_set_state(task_struct* task, task_state state)
{
    task->state = state;
    model_set_state(task->pid, state);
}
/**
 * 设置进程状态
 */
void scheduler::model_set_state(unsigned int pid, task_state state)
{
    write_model->setFilter("pid=" + QString::number(pid));
    write_model->select();
    if (write_model->rowCount() == 0) {
        qDebug() << __LINE__ << ",cannot find pid:" << pid;
        return;
    }
    QSqlRecord record = write_model->record(0);

    record.setValue(T_state, untils(state));
    write_model->setRecord(0, record);
    write_model->submitAll();
}
QString scheduler::untils(task_state state)
{
    QString str_state;
    switch (state) {
    case TASK_ACCPECT:
        str_state = "TASK_ACCPECT";
        break;
    case TASK_FINISHED:
        str_state = "TASK_FINISHED";
        break;
    case TASK_KILLED:
        str_state = "TASK_KILLED";
        break;
    case TASK_RUNNING:
        str_state = "TASK_RUNNING";
        break;
    case TASK_STOPPED:
        str_state = "TASK_STOPPED";
        break;
    case TASK_UBERRUPTIBLE:
        str_state = "TASK_UBERRUPTIBLE";
        break;
    }
    return str_state;
}

/**
 * 杀死就绪队列中/运行中的任务，并从双向链表中删除
 * @param pid
 * @return 是否找到了进程
 */
bool scheduler::kill_task(unsigned int pid)
{
    task_struct* task = nullptr;
    if (running_task->pid == pid) {
        task = running_task;
    } else {
        for (int i = 0; i < ready_queue->length(); i++) {
            if (ready_queue->at(i)->pid == pid) {
                task = ready_queue->at(i);
                break;
            }
        }
    }
    if (task != nullptr) {
        model_set_state(task, TASK_KILLED);
        return true;
    }
    qDebug() << __FILE__ << __LINE__ << ":error state value";
    return false;
}
/**
 * 阻塞当前进程，放入阻塞队列
 */
void scheduler::block_current_task()
{
    if (running_task == nullptr) {
        return;
    }
    // 设置为阻塞状态，并放入阻塞队列
    model_set_state(running_task, TASK_STOPPED);
    block_queue->append(running_task);
    running_task = nullptr;
}
/**
 * 解阻塞进程
 * @param pid 指定进程
 * @return 是否找了进程
 */
bool scheduler::unblock_task(unsigned int pid)
{
    task_struct* task;
    for (int i = 0; i < block_queue->length(); i++) {
        if (pid == block_queue->at(i)->pid) {
            if (block_queue->at(i)->state != TASK_STOPPED) {
                qDebug() << __FILE__ << __LINE__ << ":error state value";
            }
            task = block_queue->at(i);
            model_set_state(task, TASK_RUNNING);
            QMutexLocker locker(mutex);
            ready_queue->enqueue(task);
            return true;
        }
    }
    qDebug() << __LINE__ << "cannot find pid:" << pid;
    return false;
}

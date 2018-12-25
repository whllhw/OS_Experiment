#include "sched.h"
#include "pcb.h"
#include <QString>
#include <QtDebug>
#include <algorithm>
#include <connection.h>

scheduler::scheduler(QSqlTableModel* write_model, QStatusBar* statusBar)
{
    // 初始化根进程
    root_task = new task_struct("root", 0, 0);
    root_task->state = TASK_UBERRUPTIBLE;
    this->write_model = write_model;
    this->statusBar = statusBar;
}
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
    this->back_queue.append(task);
    // 放入双向链表
    if (root_task->prev == nullptr) {
        root_task->next = task;
        root_task->prev = task;
        task->prev = root_task;
        task->next = root_task;
    } else {
        root_task->prev->next = task;
        task->next = root_task;
        task->prev = root_task->prev;
        root_task->prev = task;
    }
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
bool cmp(const task_struct* s1, const task_struct* s2)
{
    return s1->priority > s2->priority;
}
/**
 * 从后备队列中取出一个高优先级的放到就绪队列
 */
void scheduler::update_ready_queue()
{
    qDebug() << "ready_queue:" << ready_queue.length();
    qDebug() << "back_queue:" << back_queue.length();
    if (ready_queue.length() > size_back_queue) {
        qDebug() << "就绪队列数量大于" << size_back_queue;
        return;
    }
    if (back_queue.length() == 0) {
        qDebug() << "后备队列数量为0";
        return;
    }
    std::sort(back_queue.begin(), back_queue.end(), cmp);
    task_struct* task = back_queue.at(0);
    back_queue.removeAt(0);
    task->state = TASK_RUNNING;
    model_set_state(task->pid, TASK_RUNNING);
    ready_queue.enqueue(task);
}
/**
 * 从就绪队列取出一个进程运行
 * 发生调度情况如下：
 * 1.当前进程时间片用完。重新入队就绪
 * 2.当前进程阻塞。入队阻塞
 * 3.当前进程被杀死。标记为杀死
 * 4.当前进程执行完毕。标记为结束
 */
void scheduler::scheduling()
{
    if (this->running_task != nullptr) {
        // 放入就绪队列末尾
        if (running_task->remain_exec_runtime <= 0) {
            model_set_state(running_task->pid, TASK_FINISHED);
            running_task = nullptr;
        } else {
            if (running_task->state != TASK_RUNNING) {
                running_task = nullptr;
            } else {
                this->ready_queue.enqueue(this->running_task);
            }
        }
    } else {
        qDebug() << "当前无运行中的进程";
        statusBar->showMessage("当前无运行中的进程");
    }
    if (this->ready_queue.empty()) {
        qDebug() << "就绪队列为空";
        return;
    }
    task_struct* task = this->ready_queue.dequeue();
    this->running_task = task;
    if (counter == 0) {
        counter = size_counter;
    }
}
/**
 * 按时间片进行轮转，可能发生调度如下：
 * 1.当前进程完成
 * 2.时间片完
 */
void scheduler::update()
{
    update_ready_queue();
    if (running_task != nullptr) {
        if (model_count_remaintime() <= 0 || --counter <= 0) {
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
        qDebug() << __LINE__ << ",找不到pid:" << pid;
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
void scheduler::model_set_state(unsigned int pid, task_state state)
{
    write_model->setFilter("pid=" + QString::number(pid));
    write_model->select();
    if (write_model->rowCount() == 0) {
        qDebug() << __LINE__ << ",找不到pid:" << pid;
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
bool scheduler::kill_task(unsigned int pid)
{
    task_struct* task = root_task->next;
    while (task != nullptr && task != root_task) {
        if (task->pid == pid) {
            // 删除双向链表
            if (task->prev) {
                task->prev->next = task->next;
                task->next->prev = task->prev;
            } else {
                qDebug() << "linked list error";
            }
            task->state = TASK_KILLED;
            model_set_state(task->pid, TASK_KILLED);
            if (running_task == task) {
                scheduling();
            }
            return true;
        }
        task = task->next;
    }
    qDebug() << "找不到pid:" << pid;
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
    running_task->state = TASK_STOPPED;
    model_set_state(running_task->pid, TASK_STOPPED);
    block_queue.append(running_task);
    running_task = nullptr;
    scheduling();
}

bool scheduler::unblock_task(unsigned int pid)
{
    task_struct* task;
    for (int i = 0; i < block_queue.length(); i++) {
        if (pid == block_queue.at(i)->pid) {
            task = block_queue.at(i);
            task->state = TASK_RUNNING;
            model_set_state(task->pid, TASK_RUNNING);
            ready_queue.enqueue(task);
            return true;
        }
    }
    qDebug() << "找不到pid:" << pid;
    return false;
}

#include "sched.h"
#include "pcb.h"
#include <QString>
#include <QtDebug>
#include <algorithm>

scheduler::scheduler()
{
    // 初始化根进程
    root_task = new task_struct("root", 0, 0);
    root_task->state = TASK_UBERRUPTIBLE;
}
/**
 * 添加一个进程并放入后备队列中
 */
bool scheduler::add_task(const char* task_name, unsigned long priority, unsigned long sum_exec_runtime)
{
    task_struct* task = new task_struct(task_name, priority, sum_exec_runtime);
    // 放入后备队列中
    task->state = TASK_ACCPECT;
    this->back_queue.append(task);
    // 放入双向链表
    if (root_task->prev == nullptr) {
        root_task->next = task;
        root_task->prev = task;
    } else {
        root_task->prev->next = task;
    }
    return true;
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
    if (ready_queue.size() > 4) {
        qDebug() << "就绪队列数量大于4";
        return;
    }
    if (back_queue.size() == 0) {
        qDebug() << "后备队列数量为0";
        return;
    }
    std::sort(back_queue.begin(), back_queue.end(), cmp);
    task_struct* task = back_queue.at(0);
    task->state = TASK_RUNNING;
    ready_queue.enqueue(task);
}
/**
 * 从就绪队列取出一个进程运行
 */
void scheduler::update()
{
    if (this->running_task != nullptr) {
        // 放入就绪队列末尾
        if (running_task->state == TASK_RUNNING) {
            this->ready_queue.enqueue(this->running_task);
        }
    } else {
        qDebug() << "当前无运行中的进程";
    }
    if (this->ready_queue.empty()) {
        qDebug() << "就绪队列为空";
        return;
    }
    task_struct* task = this->ready_queue.dequeue();
    this->running_task = task;
}
bool scheduler::kill_task(unsigned short pid)
{
    task_struct* task = root_task->next;
    while (task && task != root_task) {
        if (task->pid == pid) {
            // 删除双向链表
            if (task->prev) {
                task->prev->next = task->next;
            } else {
                qDebug() << "linked list error";
            }
            task->state = TASK_KILLED;
            if (running_task == task) {
                update();
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
    // 设置为阻塞状态，并放入阻塞队列
    this->running_task->state = TASK_STOPPED;
    this->block_queue.append(this->running_task);
    this->running_task = nullptr;
    update();
}

bool scheduler::unblock_task(unsigned short pid)
{
    task_struct* task;
    for (int i = 0; i < block_queue.size(); i++) {
        if (pid == block_queue.at(i)->pid) {
            task = block_queue.at(i);
            task->state = TASK_RUNNING;
            ready_queue.enqueue(task);
            return true;
        }
    }
    qDebug() << "找不到pid:" << pid;
    return false;
}

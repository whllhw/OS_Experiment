#include "sched.h"
#include <algorithm>
//sched_highpriority::sched_highpriority(QSqlTableModel* write_model, QStatusBar* statusBar)
//{
//    scheduler(write_model, statusBar);
//}
/**
 * 重写就绪队列更新，
 * 就绪队列按优先级排序
 */
void sched_highpriority::update_ready_queue()
{
    scheduler::update_ready_queue();
    std::sort(ready_queue->begin(), ready_queue->end(),
        [](const task_struct* s1, const task_struct* s2) { return s1->priority > s2->priority; });
}
/**
 * 高优先权非抢占式发生调度情况：
 * 1. 无运行中的进程
 * 2. 运行进程完毕
 * 3. 当前进程被杀死、阻塞
 */
void sched_highpriority::update()
{
    this->update_ready_queue();
    if (running_task == nullptr) {
        scheduling();
        return;
    }
    if (model_count_remaintime() <= 0
        || running_task->state != TASK_RUNNING) {
        scheduling();
    }
}
/**
 * 重写调度方法为优先权调度
 * 高优先权非抢占式发生调度情况：
 * 1. 无运行中的进程
 * 2. 运行进程完毕
 * 3. 进程被阻塞
 * 4. 进程被杀死
 */
void sched_highpriority::scheduling()
{
    if (running_task == nullptr) {
        //        qDebug() << "当前无运行中的进程";
        statusBar->showMessage("当前无运行中的进程");
    } else {
        if (running_task->remain_exec_runtime <= 0) {
            model_set_state(running_task->pid, TASK_FINISHED);
            running_task = nullptr;
        } else if (running_task->state == TASK_KILLED
            || running_task->state == TASK_STOPPED) {
            running_task = nullptr;
        } else {
            qDebug() << __FILE__ << __LINE__ << ":error scheduler!";
            return;
        }
    }
    if (ready_queue->empty()) {
        //        qDebug() << "就绪队列为空";
        return;
    }
    task_struct* task = this->ready_queue->dequeue();
    this->running_task = task;
}

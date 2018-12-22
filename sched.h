#ifndef SCHED_H
#define SCHED_H
#include "pcb.h"
#include <QQueue>
class scheduler {
public:
    scheduler();
    // 就绪队列
    QQueue<task_struct*> ready_queue;
    // 阻塞队列
    QList<task_struct*> block_queue;
    // 正在运行的进程
    task_struct* running_task;
    // 根进程
    task_struct* root_task;
    /**
     * 定时运行调度更新
     * 时间片轮转
     */
    void update();
    /**
     * 添加一个作业
     */
    void add_job();
    /**
     * 添加一个进程
     * @param task_name 进程名
     * @param priority  优先级
     * @return 处理结果
     */
    bool add_task(const char* task_name, unsigned long priority, unsigned long sum_exec_runtime);
    /**
     * 结束一个任务
     * @param pid
     * @return 处理结果
     */
    bool kill_task(unsigned short pid);
    /**
     * 阻塞当前进程 
     */
    void block_current_task();
    /**
     * 解除阻塞
     * @param pid
     * @return 处理结果
     */
    bool unblock_task(unsigned short pid);
};

#endif // SCHED_H

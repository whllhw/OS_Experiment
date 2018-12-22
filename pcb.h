#ifndef PCB_H
#define PCB_H
#include <QTime>

// 进程状态枚举
enum task_state {
    // 可运行状态
    TASK_RUNNING,
    // 可中断阻塞状态
    TASK_UBERRUPTIBLE,
    // 暂停态
    TASK_STOPPED,
    // 被手动结束
    TASK_KILLED,
    // 正常结束
    TASK_FINISHED
};
struct task_struct {
    // 进程名
    const char* name;
    // 进程标识
    unsigned short pid;
    // 优先级
    unsigned long priority;
    // 进程状态
    task_state state;
    // 动态优先级计数器
    unsigned long counter;
    // 组成双向链表
    struct task_struct *next, *prev;
    // 进程运行总时间
    unsigned long sum_exec_runtime;
    // 剩余运行时间
    unsigned long remain_exec_runtime;
    task_struct(const char* name, unsigned long priority, unsigned long sum_exec_runtime)
    {
        this->name = name;
        this->priority = priority;
        this->sum_exec_runtime = sum_exec_runtime;
        this->find_next_zero_pid();
    }
    ~task_struct()
    {
        free_pidmap();
    }

private:
    void free_pidmap()
    {
        // TODO
    }
    void find_next_zero_pid()
    {
        // TODO 位图法
        this->pid = (unsigned short)qrand();
    }
};
#endif // PCB_H

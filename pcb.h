#ifndef PCB_H
#define PCB_H
#include <QTime>
#include <climits>

// 进程状态枚举
enum task_state {
    // 许可状态
    TASK_ACCPECT,
    // 可运行状态
    TASK_RUNNING,
    // 暂停态
    TASK_STOPPED,
    // 正常结束
    TASK_FINISHED,
    // 被手动结束
    TASK_KILLED,
    // 可中断阻塞状态
    TASK_UBERRUPTIBLE
};
struct task_struct {
    // 进程名
    QString name;
    // 进程标识
    unsigned int pid = 0;
    // 优先级
    unsigned int priority = 0;
    // 进程状态
    task_state state;
    // 动态优先级计数器
    int counter = 0;
    // 组成双向链表
    struct task_struct *next = nullptr, *prev = nullptr;
    // 进程运行总时间
    unsigned int sum_exec_runtime = 0;
    // 剩余运行时间
    int remain_exec_runtime = 0;
    task_struct(QString name, unsigned int priority, unsigned int sum_exec_runtime)
    {
        this->name = name;
        this->priority = priority;
        this->sum_exec_runtime = sum_exec_runtime;
        this->find_next_zero_pid();
        this->remain_exec_runtime = sum_exec_runtime;
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
        this->pid = qrand();
    }
};
#endif // PCB_H

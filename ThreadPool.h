#include <queue>
#include <vector>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

class TaskBase {
public:
    TaskBase() : work_time(0) {}
    virtual void doAction() {
        // printf("this is thread: %lu\n", (unsigned long)pthread_self());
        int sleep_time = (rand() % 3);
        printf("sleep %d seconds\n", sleep_time);
        work_time = sleep_time;
        usleep(sleep_time*1000*1000);
    }
    int getWorkTime() { return work_time;}
protected:
    int work_time;
};

class ThreadPool {
public:
    struct ThreadInfo {
        pthread_t    tid;
        int          task_cnt;
        int          work_time;
        ThreadInfo()
        : tid(-1)
        , task_cnt(0)
        , work_time(0) {
        }
    };
    ThreadPool();
    ~ThreadPool();
    void run();
    void startThreadPool(int thread_nums);
    void stopThreadPool();
    void postRequest(TaskBase* task);
    bool isRunning();
    int  getQueueSize();
    void printTaskCnt();
    void setThreadInfo(pthread_t tid, int work_time);
    void clearInfoMap();

protected:
    static void* ThreadFunc(void* param);

private:
    TaskBase* takeTask();

    bool                      m_isRunning;
    pthread_mutex_t           m_mutex;
    pthread_mutexattr_t       m_mutex_attr;
    pthread_cond_t            m_cond;
    pthread_condattr_t        m_cond_attr;
    std::vector<pthread_t*>   threads;
    std::queue<TaskBase*>     task_que;
    std::map<pthread_t, ThreadInfo*>  m_thread_info;
};
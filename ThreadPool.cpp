#include "ThreadPool.h"

void* ThreadPool::ThreadFunc(void* param) {
    if (NULL != param) {
        ThreadPool* pool = reinterpret_cast<ThreadPool*>(param);
        while (pool->isRunning()) {
            TaskBase* task = pool->takeTask();
            // printf("[thread]: %lu task one\n", (unsigned long)pthread_self());
            if (NULL != task) {
                task->doAction();
                pool->setThreadInfo(pthread_self(), task->getWorkTime());
                delete task;
            }
        }
    }

    return NULL;
}

ThreadPool::ThreadPool() : m_isRunning (false) {
    pthread_mutexattr_init(&m_mutex_attr);
    pthread_mutex_init(&m_mutex, &m_mutex_attr);
    pthread_condattr_init(&m_cond_attr);
    pthread_cond_init(&m_cond, &m_cond_attr);
}

ThreadPool::~ThreadPool() {
    pthread_mutexattr_destroy(&m_mutex_attr);
    pthread_mutex_destroy(&m_mutex);
    pthread_condattr_destroy(&m_cond_attr);
    pthread_cond_destroy(&m_cond);
    clearInfoMap();
}

TaskBase* ThreadPool::takeTask() {
    TaskBase* ptask = NULL;
    while (NULL == ptask) {
        pthread_mutex_lock(&m_mutex);
        while (task_que.empty() && m_isRunning) {
            pthread_cond_wait(&m_cond, &m_mutex);
        }

        if (!m_isRunning) {
            pthread_mutex_unlock(&m_mutex);
            break;
        }
        if (task_que.empty()) {
            pthread_mutex_unlock(&m_mutex);
            continue;
        }
        ptask = task_que.front();
        task_que.pop();
        pthread_mutex_unlock(&m_mutex);
    }

    return ptask;
}

void ThreadPool::startThreadPool(int thread_nums) {
    m_isRunning = true;
    threads.reserve(thread_nums);
    for (int i = 0; i < thread_nums; ++i) {
        pthread_t* thread = reinterpret_cast<pthread_t*>(malloc(sizeof(pthread_t)));
        pthread_create(thread, NULL , &ThreadFunc, this);
        printf("create thread: %lu\n", (unsigned long)*thread);
        threads[i] = thread;
        ThreadInfo* info = new ThreadInfo();
        m_thread_info.insert(std::pair<pthread_t, ThreadInfo*>(*thread, info));
    }
}

void ThreadPool::stopThreadPool() {
    printf("stopThreadPool\n");
    m_isRunning = false;
    pthread_cond_broadcast(&m_cond);
    for (int i = 0; i < threads.size(); ++i) {
        pthread_join(*threads[i], NULL);
        free(threads[i]);
    }
}

void ThreadPool::postRequest(TaskBase* task) {
    pthread_mutex_lock(&m_mutex);
    task_que.push(task);
    pthread_mutex_unlock(&m_mutex);
    pthread_cond_signal(&m_cond);
}

bool ThreadPool::isRunning() {
    bool is_running;
    pthread_mutex_lock(&m_mutex);
    is_running = m_isRunning;
    pthread_mutex_unlock(&m_mutex);
    return is_running;
}

int ThreadPool::getQueueSize() {
    pthread_mutex_lock(&m_mutex);
    int size = task_que.size();
    pthread_mutex_unlock(&m_mutex);
    printf("current queue size is %d\n", size);
    return size;
}

void ThreadPool::printTaskCnt() {
    printf("pthread_t\ttask_cnt\twork_time\n");
    for (auto it = m_thread_info.begin(); it != m_thread_info.end(); ++it) {
        if (NULL != it->second) {
            printf("[%lu]:[%d]:[%d]\n", (unsigned long)it->first, it->second->task_cnt, it->second->work_time);
        }
    }
}

void ThreadPool::setThreadInfo(pthread_t tid, int work_time) {
    auto it = m_thread_info.find(tid);
    if (it != m_thread_info.end()) {
        if (NULL != it->second) {
            it->second->tid = tid;
            it->second->task_cnt++;
            it->second->work_time += work_time;
        } else {
            printf("ThreadInfo is NULL\n");
        }
    }
}

void ThreadPool::clearInfoMap() {
    for (auto it = m_thread_info.begin(); it != m_thread_info.end(); ++it) {
        if (NULL != it->second) {
            delete it->second;
        }
        m_thread_info.erase(it);
    }
}
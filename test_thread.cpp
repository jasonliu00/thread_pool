#include "ThreadPool.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("error arg count\n");
        exit(0);
    }
    int thread_nums = atoi(argv[1]);
    int task_nums = atoi(argv[2]);

    printf("threads: %d, tasks: %d\n", thread_nums, task_nums);
    ThreadPool tp;
    tp.startThreadPool(thread_nums);

    for (int i = 0; i < task_nums; ++i) {
        TaskBase* ptask = new TaskBase();
        tp.postRequest(ptask);
    }

    while (1) {
        if (0 == tp.getQueueSize()) {
            printf("all task have complete\n");
            tp.printTaskCnt();
            tp.stopThreadPool();
            break;
        }
        usleep(4*1000*1000);
    }

    return 0;
}
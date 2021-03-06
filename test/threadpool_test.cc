#include <xchange/base/ThreadPool.h>

#include <unistd.h>

#include <iostream>
#include <string>
#include <atomic>

using std::cout;
using std::cin;
using std::endl;
using xchange::thread::CurrentThread;
using xchange::threadPool::ThreadPool;
using xchange::threadPool::ThreadPoolEvent;
using xchange::threadPool::Task;
using xchange::threadPool::TaskEvent;
using xchange::threadPool::POOL_INIT;
using xchange::threadPool::POOL_TERMINATE;
using xchange::threadPool::TASK_START;
using xchange::threadPool::TASK_COMPLETE;

std::atomic_uint64_t counter;

void* taskMain(void *str) {
    counter.fetch_add(1);

    return str;
}

void outputResult(TaskEvent, void *p) {
    Task * taskp = static_cast<Task *>(p);

    cout << "output: " << *(std::string *)taskp->getResult() << endl;
}

void onStarup(ThreadPoolEvent, void *) {
    cout << "Thread Pool Started" << endl;
}

void onDestroy(ThreadPoolEvent, void *) {
    cout << "Thread Pool Destroyed" << endl;
}

int main(void) {
    ThreadPool pool;
    std::string a("TaskA"), b("TaskB"), c("TaskC");

    pool.on(POOL_INIT, onStarup);
    pool.on(POOL_TERMINATE, onDestroy);

    Task taskA(taskMain, &a), taskB(taskMain, &b), taskC(taskMain, &c);

    taskA.on(TASK_START, [](TaskEvent, void*) {cout << "TaskA Delivered" << endl;});
    taskA.on(TASK_COMPLETE, outputResult);
    taskB.on(TASK_START, [](TaskEvent, void*) {cout << "TaskB Delivered" << endl;});
    taskB.on(TASK_COMPLETE, outputResult);
    taskC.on(TASK_START, [](TaskEvent, void*) {cout << "TaskC Delivered" << endl;});
    taskC.on(TASK_COMPLETE, outputResult);

    pool.start();

    pool.execute(&taskA);
    pool.execute(&taskB);
    pool.execute(&taskC);

    while (1) {
        const ThreadPool::Status &stat = pool.getStatus();

        ThreadPool::checkResult();

        if (stat.busyThread == 0 && stat.unhandledTask == 0) {
            ThreadPool::checkResult();
            break;
        }
    }

    cout << counter << endl;

    return 0;
}

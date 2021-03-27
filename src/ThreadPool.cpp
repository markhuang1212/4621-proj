#include <queue>
#include <functional>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>

#include "semaphore.h"
#include "pthread.h"

#ifndef thread_pool_h
#define thread_pool_h

using namespace std;

/**
 * @brief
 * A pool of thread that execute tasks.
 * See tests/ThreadPoolTest.cpp for examples
 * 
 */
class ThreadPool
{
private:
    /**
     * @brief Count number of threads that is currently doing works
     * 
     */
    sem_t counter;

    /**
     * @brief indicate whether threads should shut themselver and stop accepting new tasks
     * 
     */
    atomic<bool> closed;

    /**
     * @brief The numbef of threads theat the ThreadPool creats upon initialization
     * 
     */
    atomic<int> num_of_threads;


    /**
     * @brief Lock for tasks queue
     * 
     */
    mutex tasks_lock;

    /**
     * @brief Storing the tasks that awaits execution
     * 
     */
    queue<function<void()>> tasks;

    vector<thread> threads;

    /**
     * @brief 
     * The one function that every thread in the thraed pool execute endlessly
     * 
     */
    void execute()
    {
        while (closed == false)
        {
            tasks_lock.lock();
            if (tasks.empty())
            {
                tasks_lock.unlock();
                this_thread::yield();
            }
            else
            {
                auto task = tasks.front();
                tasks.pop();
                sem_wait(&counter);
                tasks_lock.unlock();
                task();
                sem_post(&counter);
            }
        }
    }

public:

    ThreadPool() = delete;

    ThreadPool(int num_of_threads = 8)
        : num_of_threads(num_of_threads),
          closed(false)
    {
        sem_init(&counter, false, num_of_threads);
        for (int i = 0; i < num_of_threads; i++)
        {
            threads.emplace_back(&ThreadPool::execute, this);
        }
    }

    void addTask(const function<void()> &task)
    {
        lock_guard<mutex> lock(tasks_lock);
        tasks.push(move(task));
    }

    int numOfWaitingTasks()
    {
        lock_guard<mutex> lock(tasks_lock);
        return tasks.size();
    }

    int numOfRunningTasks()
    {
        int val;
        sem_getvalue(&counter, &val);
        return num_of_threads - val;
    }

    /**
     * @brief The function blocks until all threads are gracefully shutdown
     * 
     */
    void shutDown()
    {
        closed = true;
        for (int i = 0; i < num_of_threads; i++)
        {
            threads[i].join();
        }
    }
};

#endif
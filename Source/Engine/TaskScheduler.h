#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

using Task = std::function<void()>;

class TaskScheduler
{
  public:
    static TaskScheduler& Get();

    ~TaskScheduler();

    void AddTask(Task task);

    // This does a parallel for ona buffer, but will wait for all threads to sync before finishing
    void ParallelForSync(int32_t elementCount, std::function<void(int32_t start, int32_t end)> work);

  private:
    TaskScheduler();

    void ProcessThread();

    std::vector<std::thread> workers;
    std::queue<Task> tasks;

    std::mutex mutex;
    std::condition_variable conditionalVariable;
    const int32_t nThreads = 0;

    std::atomic<bool> isDone = false;
};
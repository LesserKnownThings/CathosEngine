#include "TaskScheduler.h"
#include <latch>
#include <thread>

TaskScheduler& TaskScheduler::Get()
{
    static TaskScheduler instace;
    return instace;
}

TaskScheduler::~TaskScheduler()
{
    isDone = true;
    conditionalVariable.notify_all();

    for (std::thread& worker : workers)
    {
        if (worker.joinable())
            worker.join();
    }
}

TaskScheduler::TaskScheduler()
    : nThreads(std::thread::hardware_concurrency())
{
    for (uint32_t i = 0; i < nThreads; ++i)
    {
        workers.emplace_back(&TaskScheduler::ProcessThread, this);
    }
}

void TaskScheduler::ParallelForSync(int32_t elementCount, std::function<void(int32_t start, int32_t end)> work)
{
    if (elementCount == 0)
        return;

    const int32_t baseChunk = elementCount / nThreads;
    const int32_t remainder = elementCount % nThreads;

    std::latch sync(nThreads);

    int32_t currentStart = 0;
    for (int32_t i = 0; i < nThreads; ++i)
    {
        int32_t chunkSize = baseChunk + (i < remainder ? 1 : 0);
        int32_t start = currentStart;
        int32_t end = start + chunkSize;
        currentStart = end;

        if (start == end)
        {
            sync.count_down();
            continue;
        }

        AddTask([&sync, start, end, &work]
                {
            work(start, end);
            sync.count_down(); });
    }

    sync.wait();
}

void TaskScheduler::AddTask(Task newTask)
{
    std::lock_guard<std::mutex> lock(mutex);
    tasks.push(newTask);
    conditionalVariable.notify_one();
}

void TaskScheduler::ProcessThread()
{
    while (!isDone)
    {
        Task task;
        {
            std::unique_lock<std::mutex> uniqueLock(mutex);
            conditionalVariable.wait(uniqueLock, [this]
                                     { return !tasks.empty() || isDone; });

            if (isDone && tasks.empty())
            {
                return;
            }

            task = std::move(tasks.front());
            tasks.pop();
        }

        if (task)
        {
            task();
        }
    }
}

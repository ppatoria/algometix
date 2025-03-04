---
title: "Understanding the Thread-per-core Pool Model"
draft: true
date: 2025-02-27T22:45:00
author: "Pralay Patoria"
---
# Understanding the Thread-per-Core Pool Model

## Introduction

The **Thread-per-Core Pool** is a highly efficient thread pool model where the number of worker threads is fixed to the number of hardware threads (logical CPU cores). This approach minimizes overhead, reduces context switching, and improves performance for CPU-bound workloads.

In this article, we will explore the design, task assignment strategies, work-stealing mechanism, and provide a simple yet complete implementation of a Thread-per-Core Pool in C++.

---

## Thread-per-Core Pool Overview

### **Key Characteristics**
1. **Threads:** The number of threads is equal to the number of CPU cores.
2. **Task Queue:** Each thread has a private task queue.
3. **Synchronization:** Minimal, since each thread primarily processes its own queue.
4. **Advantages:** Low overhead, better cache locality, and reduced context switching.
5. **Disadvantages:** May struggle with unbalanced workloads without work-stealing.

### **How Tasks Are Assigned?**
Tasks can be assigned to thread-local queues using different strategies:

- **Round-Robin:** Tasks are evenly distributed across threads in a cyclic order.
- **Work-Stealing:** If a thread runs out of tasks, it steals work from other threads.
- **Randomized Assignment:** Tasks are randomly assigned to different threads.
- **Load-Aware Assignment:** Tasks are assigned to the thread with the shortest queue.
- **Affinity-Based Assignment:** Tasks are pushed to threads based on past execution locality.

Most modern frameworks, including **oneTBB (Intel Threading Building Blocks)**, primarily use **work-stealing** to balance the workload dynamically.

---

## Thread-per-Core Pool Execution Flow

1. The pool starts with a number of threads equal to the CPU core count.
2. Each thread has its **own task queue**.
3. When a task is submitted, it is pushed to a specific thread's queue (using round-robin or another policy).
4. Each thread continuously:
   - **Takes a task from its own queue** and executes it.
   - If its queue is **empty**, it looks at other threads' queues and **steals a task** if possible.
5. This process repeats, ensuring all threads stay active and efficiently utilized.

---

## **Thread-per-Core Pool: C++ Implementation**

Here is a simple yet complete C++ implementation of a **Thread-per-Core Pool** using C++11 features like **threads, mutexes, condition variables, and atomic operations**.

```cpp
#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <optional>

class ThreadPool {
public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();

    void submit(std::function<void()> task);

private:
    void worker_thread(int index);
    std::optional<std::function<void()>> steal_task(int thief_index);

    std::vector<std::thread> workers;
    std::vector<std::queue<std::function<void()>>> task_queues;
    std::vector<std::mutex> queue_mutexes;
    std::atomic<bool> stop;
    std::vector<std::condition_variable> queue_conds;
};

ThreadPool::ThreadPool(size_t num_threads) : stop(false) {
    task_queues.resize(num_threads);
    queue_mutexes.resize(num_threads);
    queue_conds.resize(num_threads);

    for (size_t i = 0; i < num_threads; ++i) {
        workers.emplace_back(&ThreadPool::worker_thread, this, i);
    }
}

ThreadPool::~ThreadPool() {
    stop = true;
    for (auto& cond : queue_conds) cond.notify_all();
    for (auto& worker : workers) worker.join();
}

void ThreadPool::submit(std::function<void()> task) {
    static size_t index = 0;
    size_t thread_index = index++ % workers.size();

    {
        std::lock_guard<std::mutex> lock(queue_mutexes[thread_index]);
        task_queues[thread_index].push(std::move(task));
    }
    queue_conds[thread_index].notify_one();
}

void ThreadPool::worker_thread(int index) {
    while (!stop) {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(queue_mutexes[index]);
            queue_conds[index].wait(lock, [&] { return stop || !task_queues[index].empty(); });

            if (stop) return;
            if (!task_queues[index].empty()) {
                task = std::move(task_queues[index].front());
                task_queues[index].pop();
            }
        }

        if (task) {
            task();  // Execute the task
        } else {
            // Try stealing work
            task = steal_task(index);
            if (task) task();
        }
    }
}

std::optional<std::function<void()>> ThreadPool::steal_task(int thief_index) {
    for (size_t i = 0; i < workers.size(); ++i) {
        if (i == thief_index) continue;

        std::lock_guard<std::mutex> lock(queue_mutexes[i]);
        if (!task_queues[i].empty()) {
            auto task = std::move(task_queues[i].front());
            task_queues[i].pop();
            return task;
        }
    }
    return std::nullopt;
}

int main() {
    size_t num_threads = std::thread::hardware_concurrency();
    ThreadPool pool(num_threads);

    for (int i = 0; i < 10; ++i) {
        pool.submit([i] {
            std::cout << "Task " << i << " executed by thread " << std::this_thread::get_id() << "\n";
        });
    }

    std::this_thread::sleep_for(std::chrono::seconds(1)); // Allow tasks to complete
    return 0;
}
```

---

## **Conclusion**

The **Thread-per-Core Pool** is a powerful threading model designed for performance. By limiting the number of threads to match CPU cores and leveraging per-thread task queues, it minimizes synchronization overhead. The **work-stealing mechanism** ensures load balancing across threads.

### **Key Takeaways:**
✔ **Efficient CPU utilization** with minimal context switching.  
✔ **Better cache locality** by keeping tasks within the same thread.  
✔ **Work-stealing ensures load balancing** without central bottlenecks.  
✔ **Ideal for CPU-bound workloads** where tasks are independent and evenly distributed.

Would you like to explore further optimizations or different threading models? 🚀


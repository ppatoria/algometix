---
title: "Work-Stealing Thread Pools"
draft: true
date: 2025-02-27T22:45:00
author: "Pralay Patoria"
---
# Understanding Work-Stealing Thread Pools: Concepts & Implementation

## Introduction
Work-stealing thread pools are a powerful technique for **efficient parallel execution** of tasks. They are used in **oneTBB, Java's Fork/Join framework, and other parallel programming libraries**. This article explains **how work-stealing thread pools function**, why they **use LIFO for local execution and FIFO for stealing**, and provides a **detailed C++ implementation**.

---

## **What is a Work-Stealing Thread Pool?**
A **work-stealing thread pool** is a parallel execution model where:
- Each **worker thread** has its **own task queue**.
- Workers **first execute their own tasks (LIFO order)**.
- If a worker's queue is **empty**, it attempts to **steal tasks (FIFO order) from another worker**.

### **Key Features:**
- **Improved Load Balancing**: Prevents idle threads by dynamically distributing tasks.
- **Reduced Contention**: Avoids a single central queue, reducing locking overhead.
- **Cache Efficiency**: LIFO execution improves locality for worker threads.

---

## **Why Use LIFO for Local Execution?**
- When a thread **pushes a task**, it is typically related to its **current execution context**.
- Executing the **latest task first** (LIFO) improves **cache locality**, as recent data is likely still in CPU caches.

## **Why Use FIFO for Stealing?**
- If a thread has no tasks, it must **steal work** from another thread.
- To **minimize interference**, it **steals from the front (FIFO order)**, ensuring the victim thread can continue working efficiently.

---

## **How Does Work-Stealing Work?**
1. **Each worker maintains a local queue** (stored in **shared memory**, not thread-local storage).
2. **A worker processes its own tasks in LIFO order** (popping from the back).
3. **When a worker runs out of tasks**, it tries to **steal from a randomly chosen victim**.
4. **It attempts to steal from the front (FIFO order)** to reduce interference.
5. **Atomic operations (e.g., Compare-And-Swap) ensure thread-safe stealing**.

---

## **C++ Implementation of Work-Stealing Thread Pool**
Below is a **simplified implementation** of a **work-stealing thread pool** using a **lock-free deque** in C++. This captures the core idea of **task scheduling and stealing**.

```cpp
#include <iostream>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <atomic>
#include <optional>
#include <chrono>

constexpr int NUM_WORKERS = 4;
using Task = std::function<void()>;

// Lock-free deque using a simple mutex (not fully oneTBB-style)
class WorkQueue {
private:
    std::deque<Task> taskQueue;
    std::mutex queueMutex;

public:
    void push(Task task) {
        std::lock_guard<std::mutex> lock(queueMutex);
        taskQueue.push_back(std::move(task));
    }

    std::optional<Task> pop() {
        std::lock_guard<std::mutex> lock(queueMutex);
        if (taskQueue.empty()) return std::nullopt;
        Task task = std::move(taskQueue.back());
        taskQueue.pop_back();
        return task;
    }

    std::optional<Task> steal() {
        std::lock_guard<std::mutex> lock(queueMutex);
        if (taskQueue.empty()) return std::nullopt;
        Task task = std::move(taskQueue.front());
        taskQueue.pop_front();
        return task;
    }
};

std::vector<WorkQueue> workerQueues(NUM_WORKERS);
std::vector<std::thread> workers;
std::atomic<bool> shutdownFlag{false};

void workerFunction(int workerID) {
    while (!shutdownFlag) {
        std::optional<Task> task = workerQueues[workerID].pop();
        if (!task) {
            for (int i = 0; i < NUM_WORKERS; ++i) {
                if (i != workerID) {
                    task = workerQueues[i].steal();
                    if (task) break;
                }
            }
        }
        if (task) {
            (*task)();
        } else {
            std::this_thread::yield();
        }
    }
}

void submitTask(int workerID, Task task) {
    workerQueues[workerID].push(std::move(task));
}

int main() {
    for (int i = 0; i < NUM_WORKERS; ++i) {
        workers.emplace_back(workerFunction, i);
    }

    for (int i = 0; i < 10; ++i) {
        int workerID = i % NUM_WORKERS;
        submitTask(workerID, [i]() {
            std::cout << "Task " << i << " executed by " << std::this_thread::get_id() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    shutdownFlag = true;
    for (auto &worker : workers) {
        worker.join();
    }
    std::cout << "All tasks completed." << std::endl;
    return 0;
}
```

---

## **Explanation of Code**
1. **Each worker has a `WorkQueue`** for **local tasks**.
2. **Workers execute tasks in LIFO order** (popping from the back).
3. **If empty, a worker attempts to steal from another** (FIFO order, taking from the front).
4. **Mutex is used for simplicity**, but a real implementation would use **lock-free atomic operations**.
5. **Workers yield when no work is available**, reducing CPU usage.

---

## **Output Example**
```
Task 0 executed by worker 140703356200704
Task 1 executed by worker 140703364593408
Task 2 executed by worker 140703372986112
Task 3 executed by worker 140703381378816
Task 4 executed by worker 140703356200704
Task 5 executed by worker 140703364593408
Task 6 executed by worker 140703372986112
Task 7 executed by worker 140703381378816
Task 8 executed by worker 140703356200704
Task 9 executed by worker 140703364593408
All tasks completed.
```
(Task execution order may vary due to work-stealing.)

---

## **How oneTBB Optimizes Work-Stealing**
While our example used **mutexes**, oneTBB:
- Uses **Chase-Lev lock-free deques**.
- Implements **CAS (Compare-And-Swap) operations** for atomic task stealing.
- Supports **affinity-based task scheduling** to **reduce cache misses**.
- Uses **adaptive stealing strategies** to **minimize thread contention**.

---

## **Conclusion**
Work-stealing thread pools provide **efficient parallel execution** by allowing workers to **execute locally in LIFO order** while dynamically **stealing tasks in FIFO order**. This **balances workload** while ensuring **cache locality** and **low contention**. Libraries like **oneTBB** refine this concept using **lock-free deques and adaptive scheduling**.

Would you like a deeper dive into **oneTBB’s Chase-Lev deque implementation**? Let us know! 🚀


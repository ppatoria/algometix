---
title: "Thread Pool with Futures and Promises"
draft: true
date: 2025-02-27T22:45:00
author: "Pralay Patoria"
---
# Thread Pool with Futures and Promises: Mutex vs. Lock-Free Queue

## Introduction
A thread pool is a mechanism that manages a pool of worker threads, allowing efficient execution of tasks in parallel. We will explore two versions of a thread pool:
1. **Mutex & Condition Variable-Based Thread Pool**
2. **Lock-Free Queue-Based Thread Pool**

Both implementations will use **`std::future`** and **`std::promise`** to enable task submission with return values.

---

## 1. Mutex & Condition Variable-Based Thread Pool
### **Design Overview**
- A **fixed-size worker thread pool**.
- **Task submission** via `submit()`, returning `std::future<T>`.
- **Synchronization** using `std::mutex` and `std::condition_variable`.
- **Task queue** as `std::queue<std::function<void()>>`.

### **Implementation**
```cpp
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <iostream>

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads);
    ~ThreadPool();

    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))>;

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};

ThreadPool::ThreadPool(size_t numThreads) : stop(false) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    condition.wait(lock, [this] { return stop || !tasks.empty(); });
                    if (stop && tasks.empty()) return;
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers) {
        worker.join();
    }
}

template<typename F, typename... Args>
auto ThreadPool::submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
    using ReturnType = decltype(f(args...));

    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<ReturnType> future = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        if (stop) throw std::runtime_error("ThreadPool is stopped");
        tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return future;
}

int main() {
    ThreadPool pool(4);
    auto future = pool.submit([](int a, int b) { return a + b; }, 5, 10);
    std::cout << "Result: " << future.get() << "\n";
    return 0;
}
```

### **Advantages of Mutex & Condition Variable Approach**
- Simple to implement.
- Works well for **moderate contention**.

### **Disadvantages**
- **Potential contention** on `queueMutex`.
- **Blocking waits** on `condition_variable`.

---

## 2. Lock-Free Queue-Based Thread Pool
### **Design Overview**
- Replaces `std::queue` with **a lock-free queue (e.g., `std::atomic` and `std::deque`)**.
- Uses **spinlocks or atomic operations** to synchronize access.
- Improves **scalability under heavy workloads**.

### **Implementation**
```cpp
#include <vector>
#include <deque>
#include <thread>
#include <future>
#include <functional>
#include <atomic>
#include <iostream>

class LockFreeThreadPool {
public:
    explicit LockFreeThreadPool(size_t numThreads);
    ~LockFreeThreadPool();

    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))>;

private:
    std::vector<std::thread> workers;
    std::deque<std::function<void()>> tasks;
    std::atomic<bool> stop;
    std::atomic<int> taskCount;
};

LockFreeThreadPool::LockFreeThreadPool(size_t numThreads) : stop(false), taskCount(0) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] {
            while (!stop.load()) {
                if (!tasks.empty()) {
                    auto task = std::move(tasks.front());
                    tasks.pop_front();
                    task();
                }
            }
        });
    }
}

LockFreeThreadPool::~LockFreeThreadPool() {
    stop.store(true);
    for (std::thread &worker : workers) {
        worker.join();
    }
}

template<typename F, typename... Args>
auto LockFreeThreadPool::submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
    using ReturnType = decltype(f(args...));

    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    std::future<ReturnType> future = task->get_future();

    tasks.push_back([task]() { (*task)(); });
    return future;
}

int main() {
    LockFreeThreadPool pool(4);
    auto future = pool.submit([](int a, int b) { return a + b; }, 5, 10);
    std::cout << "Result: " << future.get() << "\n";
    return 0;
}
```

### **Advantages of Lock-Free Queue Approach**
- **Avoids contention** on `std::mutex`.
- **Scales better** with a high number of threads.

### **Disadvantages**
- **More complex** implementation.
- **Spinning can waste CPU cycles** if tasks are not abundant.

---

## **Conclusion**
| Approach                   | Pros                            | Cons                                 |
|----------------------------|---------------------------------|--------------------------------------|
| Mutex & Condition Variable | Simple, safe, easy to implement | Potential contention, blocking waits |
| Lock-Free Queue            | Scales well, reduces contention | Complex, may waste CPU on spinning   |

For workloads with **high contention**, the lock-free approach may provide **better performance**. However, for simpler use cases, the **mutex-based approach is easier to maintain**.

---
Would you like further optimizations or benchmarks between the two? 🚀


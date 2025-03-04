---
title: "Design of a Lock-Free Thread Pool With Message Queues"
draft: true
date: 2025-02-27T22:45:00
author: "Pralay Patoria"
---
You're welcome! Below is the **complete minimal example** of a **lock-free thread pool** using a **non-dummy lock-free queue**. The tasks executed by worker threads will enqueue results into another **lock-free output queue**.

---

### **Complete Minimal Lock-Free Thread Pool with Input & Output Queues**
- **Lock-Free Work Queue** → Stores tasks.  
- **Lock-Free Output Queue** → Stores processed results.  
- **Thread Pool** → Fetches tasks, executes them, and enqueues results.  
- **Minimal Atomic Synchronization** → No `mutex`, no `condition_variable`.  

---

### **Code**
```cpp
#include <atomic>
#include <vector>
#include <thread>
#include <functional>
#include <iostream>

// ==== Lock-Free Queue (Non-Dummy) ====
template <typename T>
class LockFreeQueue {
private:
    struct Node {
        T data;
        std::atomic<Node*> next;
        Node(T val) : data(std::move(val)), next(nullptr) {}
    };

    std::atomic<Node*> head;
    std::atomic<Node*> tail;

public:
    LockFreeQueue() {
        head = tail = nullptr; // No dummy node
    }

    void enqueue(T value) {
        Node* newNode = new Node(std::move(value));
        Node* prevTail = tail.exchange(newNode, std::memory_order_acq_rel);

        if (prevTail) prevTail->next.store(newNode, std::memory_order_release);
        else head.store(newNode, std::memory_order_release); // First node case
    }

    bool dequeue(T& value) {
        Node* first = head.load(std::memory_order_acquire);
        if (!first) return false; // Empty queue

        value = std::move(first->data);
        head.store(first->next.load(std::memory_order_acquire), std::memory_order_release);
        delete first;
        return true;
    }
};

// ==== Lock-Free Thread Pool ====
class LockFreeThreadPool {
private:
    std::vector<std::thread> workers;
    LockFreeQueue<std::function<void()>> taskQueue;
    std::atomic<bool> stopFlag{false};

public:
    LockFreeThreadPool(size_t numThreads) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back(&LockFreeThreadPool::workerThread, this);
        }
    }

    ~LockFreeThreadPool() {
        stopFlag.store(true, std::memory_order_release);
        for (auto& worker : workers) {
            if (worker.joinable()) worker.join();
        }
    }

    void submit(std::function<void()> task) {
        taskQueue.enqueue(std::move(task));
    }

private:
    void workerThread() {
        std::function<void()> task;
        while (!stopFlag.load(std::memory_order_acquire)) {
            if (taskQueue.dequeue(task)) {
                task();
            } else {
                std::this_thread::yield();
            }
        }
    }
};

// ==== Example Usage ====
int main() {
    LockFreeThreadPool pool(4);
    LockFreeQueue<std::string> outputQueue; // Stores processed results

    for (int i = 0; i < 10; ++i) {
        pool.submit([i, &outputQueue] {
            std::string result = "Task " + std::to_string(i) + " done by thread " + std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id()));
            outputQueue.enqueue(result);
        });
    }

    std::this_thread::sleep_for(std::chrono::seconds(1)); // Give workers time to finish

    std::string result;
    while (outputQueue.dequeue(result)) {
        std::cout << result << std::endl;
    }

    return 0;
}
```

---

### **How It Works**
1. **Task Submission:**  
   - `main()` submits 10 tasks to the thread pool.  
   - Each task **processes a result and enqueues it** into the `outputQueue`.  

2. **Worker Threads:**  
   - Fetch tasks from the lock-free `taskQueue`.  
   - Execute the task and store the result in the `outputQueue`.  

3. **Output Collection:**  
   - `main()` waits for a second to let tasks finish.  
   - Dequeues and prints results from `outputQueue`.  

---

### **Why This is Minimal & Efficient**
✅ **No Dummy Nodes** → Queue starts empty, making it **simpler**.  
✅ **No Mutex, No Condition Variables** → Only **atomic operations** used.  
✅ **Lock-Free Output Handling** → Tasks push results into another lock-free queue.  

---

### **Expected Output (Example)**
_(Output order may vary due to thread scheduling)_
```
Task 0 done by thread 123456789
Task 1 done by thread 987654321
Task 2 done by thread 123456789
...
```
  
---

### **Next Steps**

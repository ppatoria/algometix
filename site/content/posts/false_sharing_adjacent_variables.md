---
title: "False Sharing Scenario: When Multiple Threads Modify Adjacent Variables"
date: 2025-02-02T12:00:00+05:30
author: "Pralay Patoria"
tags: ["C++", "Performance", "Cache Optimization"]
categories: ["Low-Latency Programming"]
draft: true
---

---

False sharing occurs when multiple threads modify adjacent data stored in memory, leading to performance degradation due to unnecessary cache invalidations. In modern multi-core systems, where cache coherence protocols are crucial, minimizing false sharing becomes critical for optimizing performance in multithreaded applications.
#### **Cache Line Contention: Visual Representation**

##### **Diagram for False Sharing in Adjacent Data**
![False Sharing in Adjacent Data](/diagrams/false_sharing_adjacent_variables.png)

In this diagram, `Variable A` and `Variable B` are stored in the same cache line, causing both cores to continuously invalidate each other's cache when either variable is updated.

---

#### **Code Examples: False Sharing in Independent and Struct Variables**


##### **1. False Sharing with Independent Variables**

```cpp
#include <iostream>
#include <thread>
const int NUM_ITER = 10000000;

int a = 0; // Modified by Thread 1
int b = 0; // Modified by Thread 2

void threadFunc1() {
    for (int i = 0; i < NUM_ITER; ++i) {
        a++;
    }
}

void threadFunc2() {
    for (int i = 0; i < NUM_ITER; ++i) {
        b++;
    }
}

int main() {
    std::thread t1(threadFunc1);
    std::thread t2(threadFunc2);
    t1.join();
    t2.join();
    std::cout << "Final values: " << a << ", " << b << std::endl;
}
```
In this example, a and b are adjacent in memory, possibly on the same cache line. As thread 1 modifies a and thread 2 modifies b, the cache line containing both variables must be invalidated and reloaded, causing excessive cache coherence traffic and negatively affecting performance.



##### **2. False Sharing with Struct Variables**
```cpp
#include <iostream>
#include <thread>
const int NUM_THREADS = 2;
const int NUM_ITER = 10000000;

struct SharedData {
    int a;  // Modified by Thread 1
    int b;  // Modified by Thread 2
} data;

void threadFunc1() {
    for (int i = 0; i < NUM_ITER; ++i) {
        data.a++;  // This will cause false sharing with data.b
    }
}

void threadFunc2() {
    for (int i = 0; i < NUM_ITER; ++i) {
        data.b++;  // This will cause false sharing with data.a
    }
}

int main() {
    std::thread t1(threadFunc1);
    std::thread t2(threadFunc2);
    t1.join();
    t2.join();
    std::cout << "Final values: " << data.a << ", " << data.b << std::endl;
}
```
Similarly, in this example, data.a and data.b are adjacent in memory, possibly on the same cache line. As thread 1 modifies data.a and thread 2 modifies data.b, the cache line containing both member variables must be invalidated and reloaded, causing excessive cache coherence traffic and negatively affecting performance.

---

#### **Optimizing for Performance: Mitigating False Sharing**

##### **1. Use Alignment to Separate Variables**
###### **a. Stack or Global Variables with `alignas`**

```cpp
alignas(64) int a = 0; 
alignas(64) int b = 0; 
```

###### **b. Heap Allocation with Alignment**

```cpp
int* a = static_cast<int*>(std::aligned_alloc(64, 64));
int* b = static_cast<int*>(std::aligned_alloc(64, 64));
```
**Note:** std::aligned_alloc is available only in C++17 and later versions. If working with an earlier C++ version, consider using posix_memalign or similar alternatives for heap alignment.
> **Prevents automatic adjacent placement in memory.***

##### **2. Use Padding to Separate Variables**

```cpp
struct PaddedInt {
    int value;
    char padding[60]; // Assuming a 64-byte cache line
};

PaddedInt a;
PaddedInt b;
```

```cpp
struct alignas(64) SharedData {
    int a;
    char padding[60];  // Padding to ensure a and b do not share the same cache line
    int b;
};
```

> **Forces `a` and `b` to be allocated in different cache lines, reducing contention.**

##### **3. Using Thread-Local Storage (`thread_local`)**
```cpp
thread_local int a;
thread_local int b;
```
Using thread_local ensures that each thread gets its own instance of a and b, stored in thread-local storage, which prevents false sharing as the variables are not shared between threads.

---

### **Key Takeaways**
- False sharing occurs when multiple threads modify adjacent variables that share the same cache line, leading to performance degradation.
- The problem is the same whether the variables are independent or part of a struct.
- Mitigation strategies include alignment, padding, splitting variables, and using thread-local storage.
---

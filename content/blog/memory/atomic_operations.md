---
title: "Understanding Atomic Operations and Memory Fences in C++"
draft: true
date: 2025-02-27T22:45:00
author: "Pralay Patoria"
---
# Understanding Atomic Operations and Memory Fences in C++

## **Introduction**
When working with multithreading in C++, ensuring proper synchronization is critical to prevent race conditions and maintain data consistency. The `std::atomic` class provides several operations to achieve atomicity, ensuring that updates to shared variables are not interrupted by other threads. In this article, we will explore key atomic operations, including **Read-Modify-Write (RMW) operations**, **compare-and-exchange**, **fetch-and-modify**, and **memory fences** with detailed explanations and examples.

---

## **Understanding Read-Modify-Write (RMW) Operations**

### **Are All Writes RMW?**
Not all writes are RMW. A simple atomic store operation (`atomic_var.store(value)`) is **not** considered an RMW operation because it simply writes a new value without depending on the existing value.

However, operations like `fetch_add()`, `fetch_sub()`, and `compare_exchange_weak/strong()` **are RMW** because they read the existing value, modify it, and write it back atomically.

### **Example of RMW Operation**
```cpp
std::atomic<int> counter(0);

void increment() {
    counter.fetch_add(1, std::memory_order_relaxed); // RMW: Read, Modify, Write
}
```
In this case, `fetch_add(1)` ensures that even if multiple threads execute `increment()`, the counter remains correct.

---

## **Compare-and-Exchange (CAS) Operations**
Compare-and-exchange is a powerful operation used in lock-free programming. It checks whether the current value matches an expected value and, if so, updates it with a new value atomically.

### **Compare-and-Exchange Weak vs. Strong**
1. **`compare_exchange_weak()`** may fail spuriously due to weak memory ordering or speculative execution, so it is often used in a loop.
2. **`compare_exchange_strong()`** does not fail spuriously and only fails when the expected value does not match the current value.

### **Example: CAS with Weak Compare-Exchange**
```cpp
std::atomic<int> counter(0);

void increment() {
    int expected, desired;
    do {
        expected = counter.load();
        desired = expected + 1;
    } while (!counter.compare_exchange_weak(expected, desired));
}
```
The loop ensures that even if `compare_exchange_weak()` fails spuriously, it retries until the operation succeeds.

---

## **Fetch-and-Modify Operations**
Fetch-and-modify operations, such as `fetch_add()`, `fetch_sub()`, `fetch_and()`, `fetch_or()`, etc., are RMW operations that atomically modify a variable and return its previous value.

### **Example: Using `fetch_add()` Correctly**
```cpp
std::atomic<int> counter(0);

void increment() {
    counter.fetch_add(1, std::memory_order_relaxed);
}
```
This ensures atomicity, unlike `counter = counter + 1;`, which is **not atomic** and may lead to race conditions.

---

## **Memory Fences (Standalone Operations)**
Memory fences (`std::atomic_thread_fence`) enforce memory ordering constraints without modifying a specific atomic variable. They are used for low-level memory synchronization.

### **Example: Producer-Consumer with Memory Fences**
**Incorrect Implementation:**
```cpp
std::atomic<bool> ready(false);
int data = 0;

void producer() {
    data = 42;  // Modify non-atomic variable
    ready.store(true);  
    std::atomic_thread_fence(std::memory_order_release);  // 🚨 Incorrect position!
}

void consumer() {
    std::atomic_thread_fence(std::memory_order_acquire);  // 🚨 Incorrect position!
    while (!ready.load());
    std::cout << "Data: " << data << std::endl;
}
```
❌ **Problem:** The release fence comes *after* modifying `ready`, and the acquire fence comes *before* checking `ready`. This could lead to **stale reads of `data`** due to CPU reordering.

### **Corrected Implementation:**
```cpp
std::atomic<bool> ready(false);
int data = 0;

void producer() {
    data = 42;  
    std::atomic_thread_fence(std::memory_order_release);  // ✅ Ensures `data` is visible before `ready`
    ready.store(true, std::memory_order_relaxed);
}

void consumer() {
    while (!ready.load(std::memory_order_relaxed));  // ✅ Waits for `ready = true`
    std::atomic_thread_fence(std::memory_order_acquire);  // ✅ Ensures `data = 42` is visible
    std::cout << "Data: " << data << std::endl;
}
```
✔ **Fix:** The release fence ensures that `data` is stored **before** `ready = true`, and the acquire fence ensures that once `ready` is observed as `true`, `data` is properly read.

---

## **Why Is `while (!ready);` a Non-Atomic Flag Check?**
Using a non-atomic flag like `bool ready` without `std::atomic` introduces potential issues:

1. **Compiler Optimization:** The compiler may cache `ready` in a register, preventing updates from other threads from being seen.
2. **CPU Reordering:** Some CPU architectures may reorder instructions, leading to inconsistent reads.

✅ **Solution:** Always use `std::atomic<bool> ready;` with `load()` and `store()` for proper thread synchronization.

---

## **Key Takeaways**
| Feature | Non-Atomic (`bool ready`) | Atomic (`std::atomic<bool> ready`) |
|---------|----------------|-------------------|
| Compiler optimizations | May cache `ready` (loop may never exit) | Always fetches the latest value |
| CPU reordering | May read `ready = true` but stale `data` | Ensures correct ordering with `memory_order_acquire` |
| Thread safety | ❌ Not safe (race condition possible) | ✅ Safe (atomic operations ensure visibility) |

🔹 **Use `std::atomic<bool> ready;` for proper synchronization**  
🔹 **Use `memory_order_acquire/release` to enforce proper memory ordering**  
🔹 **Use `fetch_add()`, `compare_exchange_weak/strong()` for atomic updates**

---

## **Conclusion**
Atomic operations in C++ are powerful but must be used correctly to ensure safe multithreading. **RMW operations**, **compare-and-exchange**, and **fetch-and-modify** provide atomic updates, while **memory fences** ensure proper ordering. Misplacing memory fences can introduce subtle bugs, so always ensure that acquire fences are used **after** loads and release fences are used **before** stores.

Would you like a deep dive into lock-free data structures using these principles? 🚀


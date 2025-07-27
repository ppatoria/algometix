# Creating a Recurring Timer in C++: A Step-by-Step Tutorial

## 1. Introduction

A recurring timer is useful when you need to execute a task repeatedly at a fixed time interval (e.g., every second). In C++, a common way to implement such a timer is by using a separate thread that waits for a specified duration, then executes a callback function. This tutorial shows you how to build a thread-safe recurring timer using a dedicated thread, a condition variable, and proper synchronization.

---

## 2. Implementing the Timer

Below is the complete implementation of a thread-safe recurring timer:

```cpp
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <functional>
#include <iostream>

class Timer {
private:
    std::thread timer_thread;
    bool isRunning;
    std::mutex mutex;
    std::condition_variable cv;

public:
    Timer() : isRunning(false) {}

    // Prevent copying to avoid issues with the timer thread.
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    // Start the timer with a recurring interval.
    void start(std::chrono::milliseconds ms, std::function<void()> callback) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (isRunning) return;  // Prevent starting twice.
            isRunning = true;
        }

        // Create a new thread that runs the timer loop.
        timer_thread = std::thread([this, ms, callback]() {
            while (true) {
                {
                    // Lock the mutex and wait for the specified duration.
                    // If notified (stop requested), the condition returns no_timeout.
                    std::unique_lock<std::mutex> lock(mutex);
                    if (cv.wait_for(lock, ms) == std::cv_status::no_timeout) {
                        break;  // Stop signal received.
                    }
                }
                // Mutex is released here before the callback is executed.
                callback();
            }
        });
    }

    // Stop the timer safely.
    void stop() {
        {
            std::lock_guard<std::mutex> lock(mutex);
            isRunning = false;
            cv.notify_one();  // Wake up the timer thread to exit.
        }
        if (timer_thread.joinable()) {
            timer_thread.join();
        }
    }

    ~Timer() {
        stop();  // Ensure proper cleanup on destruction.
    }
};

int main() {
    Timer tm;
    tm.start(
        std::chrono::milliseconds(1000),
        []() { std::cout << "Timer triggered" << std::endl; }
    );
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));  // Let the timer run for 10 seconds.
    return 0;
}
```

---

## 3. Design Overview

Now that we have seen the implementation, let's break down the design choices and why they were made.

### 3.1. Why Use Explicit `start()` and `stop()` Methods?

#### Flexibility:
Separating the timer’s start and stop functionality gives the user control over when the timer begins and ends. Relying only on a constructor and destructor limits when the timer is active.

#### Restartability:
With explicit start/stop functions, you can choose to restart the timer later if needed.

### 3.2. Why Use a Condition Variable Instead of Only an Atomic Boolean?

#### Efficient Waiting:
A condition variable allows a thread to sleep without busy-waiting. This is important for efficiency because the thread won’t consume CPU cycles while waiting for the timeout.

#### Clean Shutdown:
When stopping the timer, you can notify the waiting thread immediately, ensuring that it wakes up and terminates promptly.

#### Separation of Concerns:
An atomic boolean could signal when to stop, but it doesn’t provide an efficient waiting mechanism. The condition variable is the proper tool for waiting until either the timeout expires or a stop signal is received.

### 3.3. How to Ensure `start()` Is Called Only Once

#### Using a Flag:
A simple `bool isRunning` flag, protected by a mutex, can be used to ensure that the timer is not started multiple times concurrently.

#### Why Not `std::call_once`:
While `std::call_once` guarantees that a function is executed only once, it is typically used for one-time initialization (like singleton patterns). Using it here would:

- **Prevent Restartability:** Once started, the timer could never be restarted.
- **Add Overhead:** `std::call_once` introduces extra synchronization overhead that is unnecessary when a simple flag (protected by a mutex) works perfectly well.

---

## 4. Conclusion

In this tutorial, you learned how to create a recurring timer in C++ using a separate thread, condition variables, and proper synchronization techniques. We discussed:
- **Why explicit `start()` and `stop()` methods are used instead of relying solely on constructors and destructors.**
- **The benefits of a condition variable for efficient waiting and clean shutdown over using an atomic boolean alone.**
- **How to ensure that the timer is started only once using a simple flag and mutex.**
- **Why `std::call_once` isn’t suitable for a restartable timer due to design constraints and extra overhead.**

This design provides a robust, thread-safe timer that can be easily integrated into your C++ applications. Enjoy building your applications with this recurring timer!


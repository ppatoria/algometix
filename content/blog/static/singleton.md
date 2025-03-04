---
title: "Singleton Patterns in C++"
date: 2025-02-16T22:45:00
author: "Pralay Patoria"
---

# A Deep Dive into Singleton Patterns in C++

## Introduction
The **Singleton Pattern** is a widely used design pattern that ensures a class has only one instance while providing a global point of access to it. However, achieving a **thread-safe**, **destruction-order-safe**, and **efficient** singleton in C++ comes with challenges. In this article, we will cover:

- The concept of **static variables** and their storage in C++.
- C++ **guarantees** regarding static variable initialization.
- **Meyers' Singleton** and its limitations.
- **Destruction-order issues** in Meyers' Singleton.
- **std::call_once & std::once_flag** as a complete solution.
- An **under-the-hood** look at `std::call_once` and `std::once_flag`.

---

## **1. Understanding Static Variables and Their Storage**
C++ allows defining **static variables** in three contexts:
1. **Local Static Variables** (inside functions).
2. **Static Member Variables** (inside classes).
3. **Global Static Variables** (outside functions and classes).

### **1.1 Local Static Variables**
Local static variables are declared inside a function and have the following properties:
- **Storage**: Allocated in the **data segment** (not on the heap or stack).
- **Initialization**: Initialized only once before their first use.
- **Lifetime**: Extends until program termination.
- **Scope**: Limited to the function in which they are declared.

#### **Example: Local Static Variable**
```cpp
#include <iostream>

void foo() {
    static int x = 10;  // Stored in the data segment, initialized once
    x++;
    std::cout << x << "\n";
}

int main() {
    foo();  // Output: 11
    foo();  // Output: 12
    foo();  // Output: 13
    return 0;
}
```
- **Behavior**: The variable `x` is initialized only once when `foo()` is first called. Subsequent calls to `foo()` increment the same instance of `x`.
- **Lifetime**: `x` persists until the program terminates.
- **Scope**: `x` is only accessible within `foo()`.

---

### **1.2 Static Member Variables**
Static member variables are declared inside a class but defined outside the class. They have the following properties:
- **Storage**: Allocated in the **data segment**.
- **Initialization**: Zero-initialized before `main()` starts.
- **Lifetime**: Extends until program termination.
- **Scope**: Shared across all instances of the class.

#### **Example: Static Member Variable**
```cpp
#include <iostream>

class Example {
public:
    static int count;  // Declaration of static member variable

    Example() { count++; }

    static void showCount() {
        std::cout << "Count: " << count << std::endl;
    }
};

// Definition of static member variable (outside the class)
int Example::count = 0;

int main() {
    Example::showCount(); // Prints: Count: 0
    Example e1, e2, e3;
    Example::showCount(); // Prints: Count: 3
    return 0;
}
```
- **Behavior**: The static member variable `count` is shared across all instances of the `Example` class. It is initialized to `0` before `main()` starts and persists until program termination.
- **Lifetime**: `count` lives until the program exits.
- **Scope**: `count` is accessible through the class name (`Example::count`) or any instance of the class.

---

### **1.3 Global Static Variables**
Global static variables are declared outside functions and classes. They have the following properties:
- **Storage**: Allocated in the **data segment**.
- **Initialization**: Zero-initialized before `main()` starts.
- **Lifetime**: Extends until program termination.
- **Scope**: Limited to the translation unit (file) in which they are declared.

#### **Example: Global Static Variable**
```cpp
#include <iostream>

static int globalVar = 42;  // Global static variable

void showValue() {
    std::cout << "GlobalVar: " << globalVar << std::endl;
}

int main() {
    showValue();  // Output: GlobalVar: 42
    return 0;
}
```
- **Behavior**: The global static variable `globalVar` is initialized to `42` before `main()` starts and persists until program termination.
- **Lifetime**: `globalVar` lives until the program exits.
- **Scope**: `globalVar` is accessible only within the file in which it is declared due to the `static` keyword.

---

## **2. Key Features of Static Variables**
1. **Stored in the Data Segment**: Unlike normal member variables, static variables are stored in the **static/global data segment**.
2. **Shared Across All Instances**: A static member variable is shared across all instances of a class.
3. **Zero-Initialized Before Main Begins**: Static variables are initialized to **zero** before `main()` starts.
4. **Lifetime = Entire Program**: They live until the program exits.
5. **Thread Safety Issues**: If a static variable is explicitly assigned a value after program startup (e.g., in a function), it may require synchronization to prevent race conditions.

---

## **3. Thread Safety Issue in Static Members**
If a static **member variable** is assigned a value later in a function (not inside a local static function scope), it may require explicit locking to prevent race conditions.

#### **Example: Thread-Unsafe Static Member Initialization**
```cpp
class Singleton {
private:
    static Singleton* instance;
    Singleton() {}
public:
    static Singleton* getInstance() {
        if (!instance) {  // Multiple threads can enter here
            instance = new Singleton();
        }
        return instance;
    }
};
Singleton* Singleton::instance = nullptr;  // Zero initialized
```
- **Race Condition**: Multiple threads may simultaneously detect `instance` as `nullptr` and create multiple instances.
- **Solution**: Use `std::call_once` or a local static variable (Meyers' Singleton).

---

## **4. Meyers' Singleton and C++ Guarantees**
**Meyers' Singleton** is an elegant way to implement a **thread-safe** singleton using a **local static variable** inside a function.

### **Implementation**
```cpp
#include <iostream>

class Singleton {
public:
    static Singleton& getInstance() {
        static Singleton instance;  // Local static variable, thread-safe
        return instance;
    }

    void log(const std::string& msg) { std::cout << msg << "\n"; }

private:
    Singleton() { std::cout << "Singleton Initialized\n"; }
    ~Singleton() { std::cout << "Singleton Destroyed\n"; }
};

int main() {
    Singleton::getInstance().log("Hello from Singleton");
    return 0;
}
```

### **Why Is This Thread-Safe?**
Since **C++11**, the standard guarantees that **function-local static variables** are initialized in a **thread-safe manner**. The first thread that accesses `getInstance()` initializes the instance, ensuring **no race conditions**.

### **How Does C++ Ensure Thread Safety?**
- The **compiler generates atomic operations** that ensure only one thread performs initialization.
- Since **static local variables** are stored in the **data segment**, their initialization is managed separately, making it different from objects on the heap or stack.
- Unlike **heap or stack allocations**, which require explicit synchronization, **static local variables use compiler-generated synchronization mechanisms.**

---

## **5. Issues with Meyers' Singleton**
While **Meyers' Singleton** is thread-safe and efficient, it has two significant limitations:
1. **Static Initialization Order Fiasco**.
2. **Destruction Order Issue**.

---

### **5.1 Static Initialization Order Fiasco**
The **Static Initialization Order Fiasco** occurs when **global or static objects** depend on each other across different translation units (source files). If one object is accessed before it is initialized, it leads to **undefined behavior**.

#### **Example: Static Initialization Order Fiasco with Meyers' Singleton**
```cpp
// File: singleton.h
#ifndef SINGLETON_H
#define SINGLETON_H

#include <iostream>

class Singleton {
public:
    static Singleton& getInstance() {
        static Singleton instance;  // Meyers' Singleton
        return instance;
    }

    void log(const std::string& msg) { std::cout << "Log: " << msg << "\n"; }

private:
    Singleton() { std::cout << "Singleton Initialized\n"; }
    ~Singleton() { std::cout << "Singleton Destroyed\n"; }
};

#endif // SINGLETON_H
```

```cpp
// File: logger.h
#ifndef LOGGER_H
#define LOGGER_H

#include "singleton.h"

class Logger {
public:
    Logger() {
        // Accessing Singleton during construction
        Singleton::getInstance().log("Logger Initialized");
    }

    void log(const std::string& msg) {
        Singleton::getInstance().log(msg);
    }
};

extern Logger globalLogger;  // Declare a global Logger instance

#endif // LOGGER_H
```

```cpp
// File: logger.cpp
#include "logger.h"

Logger globalLogger;  // Define the global Logger instance
```

```cpp
// File: main.cpp
#include "logger.h"

int main() {
    globalLogger.log("Hello from main!");
    return 0;
}
```

#### **What Happens?**
1. The `globalLogger` object is a **global static object** defined in `logger.cpp`.
2. During the **initialization phase** of `globalLogger`, its constructor calls `Singleton::getInstance()` to log a message.
3. However, the **Singleton instance** might not yet be initialized because the **initialization order of global objects across translation units is undefined**.
4. If the `Singleton` instance is not initialized when `globalLogger` is constructed, accessing it leads to **undefined behavior**.

---

### **5.2 Destruction Order Issue**
The **Destruction Order Issue** occurs when a static or global object is accessed during the **destruction phase** of another static/global object. If the object being accessed has already been destroyed, it leads to **undefined behavior**.

#### **Example: Destruction Order Issue**
```cpp
#include <iostream>

class Singleton {
public:
    static Singleton& getInstance() {
        static Singleton instance;
        return instance;
    }
    void log(const std::string& msg) { std::cout << msg << "\n"; }
};

class Application {
public:
    ~Application() {
        Singleton::getInstance().log("Application is shutting down...");
    }
};

Application app;  // Global instance

int main() {
    return 0;
}
```
- **Issue**: The `Application` destructor calls `Singleton::getInstance()`, but `Singleton` might have already been destroyed, leading to undefined behavior.

---

## **6. Using `std::call_once` and `std::once_flag`**
To provide **explicit control** over thread safety, initialization order, and destruction order, C++11 introduced `std::call_once` and `std::once_flag`.

---

### **6.1 Thread-Safe Singleton Using `std::call_once`**
```cpp
#include <iostream>
#include <mutex>

class Singleton {
public:
    static Singleton& getInstance() {
        std::call_once(initFlag, []() {
            instance = new Singleton();
        });
        return *instance;
    }

    void log(const std::string& msg) { std::cout << msg << "\n"; }

private:
    Singleton() { std::cout << "Singleton Initialized\n"; }
    ~Singleton() { std::cout << "Singleton Destroyed\n"; }

    static Singleton* instance;
    static std::once_flag initFlag;
};

Singleton* Singleton::instance = nullptr;
std::once_flag Singleton::initFlag;

int main() {
    Singleton::getInstance().log("Safe Singleton!");
    return 0;
}
```

---

### **6.2 Under-the-Hood Look at `std::call_once` and `std::once_flag`**
#### **Assumed Implementation (Simple Version)**
```cpp
#include <iostream>
#include <atomic>
#include <thread>

class once_flag {
public:
    std::atomic<int> state{0};  // 0 = not executed, 1 = in progress, 2 = done
};

template <typename Callable>
void call_once(once_flag& flag, Callable func) {
    int expected = 0;  // Initial state

    // Try to set the state to 1 (execution in progress)
    if (flag.state.compare_exchange_strong(expected, 1, std::memory_order_acquire)) {
        // First thread executes the function
        func();
        flag.state.store(2, std::memory_order_release);  // Mark as executed
    } else {
        // Other threads must wait until state is 2 (done)
        while (flag.state.load(std::memory_order_acquire) != 2) {
            std::this_thread::yield();  // Allow other threads to run
        }
    }
}
```

#### **How This Works**
1. The **first thread** that calls `std::call_once` sets the `state` to `1` (execution in progress) and executes the function.
2. Other threads wait until the `state` becomes `2` (execution completed).
3. Once the function is executed, the `state` is set to `2`, and all waiting threads proceed without re-executing the function.

---

## **Conclusion**
- **Meyers' Singleton** is a great default choice if **destruction order is not an issue**.
- If **global/static objects depend on the singleton**, use `std::call_once` for **explicit control over destruction order**.

--- 

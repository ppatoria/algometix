---
title: "Understanding Load Buffers and Parallel Execution in CPUs"
draft: true
date: 2025-02-16T22:45:00
author: "Pralay Patoria"
---

## **Flow of a Load Request**
```plantuml
@startuml
title Optimized CPU Load Request - Activity Diagram

start
:CPU issues Load Instruction;
:LSU (Load-Store Unit) handles request;

repeat
  if (Data in Cache (i.e., L1, L2, L3... etc)?) then (Yes)
    :Load from Cache to Load Buffer;
    :Send to CPU Register;
    stop
  else (No)
    :Move to Next Cache Level;
  endif
repeat while (Is still caches remain to check?)

if (Single-Core?) then (Yes)
  :Request from RAM via IMC;
  :Store in Cache, Load Buffer;
  :Send to CPU Register;
  stop
else (No)
  :Cache Coherence Check (Multi-core);
  if (Data in another core’s cache?) then (Yes)
    :Retrieve data from other core;
    :Store in Cache, Load Buffer;
    :Send to CPU Register;
    stop
  else (No)
    :Request from RAM via IMC;
    :Store in Cache, Load Buffer;
    :Send to CPU Register;
    stop
  endif
endif
@enduml
```
When the CPU needs a value from memory, it follows this sequence:

1. **Instruction Dispatch:**  
   - The CPU issues a **load instruction** (e.g., `MOV` in x86, `LD` in ARM).
   - The Load-Store Unit (LSU) handles memory accesses.

2. **Checking the L1 Cache (Fastest Level)**
   - The LSU first checks the **L1 Data Cache (L1D)**.
   - If the data is **present (cache hit)**, it is sent to the **Load Buffer** and then to the CPU register.
   - If **not present (cache miss)**, the request moves to L2.

3. **Checking the L2 Cache (and L3 if needed)**
   - If the data isn’t in L1, the request goes to **L2 cache**.
   - If it’s still a **miss**, it checks **L3 cache (if available).**
   - If found at any cache level, the data is sent to the Load Buffer.

4. **Cache Coherence Check (Multi-core systems)**
   - If the CPU is multi-core, the **cache coherence protocol (MESI, MOESI, etc.)** ensures **no other core has a more recent version of the data.**  
   - This involves checking the **cache state flags** to ensure consistency.

5. **Fetching from RAM (Last Resort)**
   - If the data is **not in any cache**, the **Memory Controller (IMC) requests it from RAM**.
   - The data is fetched into the cache **before reaching the Load Buffer.**
   - It **never** loads directly from RAM to the Load Buffer (except with special non-temporal loads).

6. **Storing in Load Buffer and Register**
   - The data **enters the Load Buffer** while waiting to be used.
   - It is then **written into the CPU register for execution.**

## **What is a Load Buffer?**
A **load buffer** is a critical CPU component that temporarily holds memory load requests while waiting for data to arrive from the cache or RAM. It allows the CPU to continue executing instructions without blocking on memory access.

### **Block Diagram of Load Buffer Interaction**
Below is a **PlantUML diagram** illustrating the interaction between the **CPU, Load Buffer, Cache, and RAM**:

```plantuml
@startuml

node CPU {
  [Instruction Execution]
}

node "Load Buffer" {
  [Pending Loads]
}

node Cache {
  [L1 Cache]
  [L2 Cache]
}

node RAM {
  [Main Memory]
}

CPU --> "Load Buffer": Sends Load Requests
"Load Buffer" --> Cache: Requests Data
Cache --> "Load Buffer": Provides Cached Data
"Load Buffer" --> CPU: Delivers Data when Ready
Cache --> RAM: Fetches Data if Cache Miss
RAM --> Cache: Supplies Data

@enduml
```

## **Why Do We Need Load Buffers?**
Load buffers exist to **hide memory latency** and enable efficient execution. Without them, CPUs would stall every time a memory request was made.

### **Key Benefits of Load Buffers:**

### **1. Latency Hiding**
**Problem:** Fetching data from RAM can take hundreds of CPU cycles.

**Solution:** The load buffer holds pending loads and lets the CPU execute **other instructions** while waiting.

**Example:**
```cpp
volatile int global_var = 42;
int main() {
    int a, b;
    a = global_var;  // Load request sent
    b = a * 2;       // Dependent on 'a', must wait
    int c = 100;     // Independent, can execute now while Load request completes in parallel
}
```
- The CPU starts fetching `global_var`.
- While waiting, it executes `c = 100`.
- Once `a` is ready, it computes `b = a * 2`.

### **2. Out-of-Order Execution**
**Problem:** Sequential execution would waste cycles waiting for loads.

**Solution:** Out-of-order execution allows independent instructions to execute **before** memory loads complete.

**Example:**
```cpp
volatile int global_var = 42;
int main() {
    int a, b, c;
    a = global_var;  // Load (waiting)
    c = 50;          // Independent, executes first
    b = a + 5;       // Must wait for 'a'
}
```
- `a = global_var` waits for memory.
- `c = 50` executes **before** `b = a + 5`.

### **3. Memory-Level Parallelism (MLP)**
**Problem:** If multiple loads execute sequentially, execution slows down.

**Solution:** CPUs issue **multiple loads** in parallel using multiple load buffers.

**Example:**
```cpp
volatile int x = 10, y = 20;
int main() {
    int a = x; // Load 1
    int b = y; // Load 2 (issued in parallel)
    int c = a + b;
}
```
- Both `x` and `y` are fetched in parallel, reducing stalls.

### **4. Data Staging**
**Problem:** Some computations need frequently accessed data.

**Solution:** The **load buffer temporarily stores recently loaded data** for fast access.

**Example:**
```cpp
volatile int arr[100];
int main() {
    int a = arr[5];  // Load 1
    int b = arr[5];  // Uses staged data (fast access)
}
```
- The second access to `arr[5]` is **instantaneous** due to data staging.


## **Conclusion**
Load buffers are essential for modern CPUs, enabling **latency hiding, out-of-order execution, memory parallelism, and data staging**. Understanding them allows developers to write optimized, high-performance code.



# Performance counter Stats:

```bash
bash> perf stat ./cache_friendly
```

``` bash
Performance counter stats for './cache_friendly':

             22.16 msec task-clock                       #    0.975 CPUs utilized             
                 6      context-switches                 #  270.758 /sec                      
                 5      cpu-migrations                   #  225.632 /sec                      
             2,122      page-faults                      #   95.758 K/sec                     
        66,957,848      cycles                           #    3.022 GHz                       
        21,933,617      instructions                     #    0.33  insn per cycle            
         3,864,534      branches                         #  174.392 M/sec                     
            18,917      branch-misses                    #    0.49% of all branches           

       0.022738693 seconds time elapsed

       0.016393000 seconds user
       0.005786000 seconds sys
```
Here's a **detailed breakdown** of each column in the `perf stat` output. You can use this as a **reference** whenever analyzing `perf` results, irrespective of the source code.

---

## **Understanding `perf stat` Output**

### **Core Metrics**
| **Metric**                       | **Value** (Example) | **Description**                                                                                                                                      |
|----------------------------------|---------------------|------------------------------------------------------------------------------------------------------------------------------------------------------|
| **Task Clock**                   | `22.16 msec`        | The total time the process spent running on **CPU cores**. Helps in understanding execution time.                                                    |
| **CPUs Utilized**                | `0.975`             | The fraction of CPU resources used during execution (`1.0` means fully utilizing one CPU core).                                                      |
| **Context Switches**             | `6`                 | The number of times the OS **switched execution** from this process to another. Higher values indicate interference from other tasks.                |
| **Context Switch Rate**          | `270.758 /sec`      | Context switches per second. Lower is better for performance.                                                                                        |
| **CPU Migrations**               | `5`                 | How often the process switched between **different CPU cores**. Too many migrations can harm cache efficiency.                                       |
| **CPU Migration Rate**           | `225.632 /sec`      | Migrations per second. High values indicate OS scheduling overhead.                                                                                  |
| **Page Faults**                  | `2,122`             | Number of **memory accesses** that were not found in RAM and had to be retrieved from disk or swapped memory.                                        |
| **Page Fault Rate**              | `95.758 K/sec`      | How often the program is causing page faults. High values can slow performance.                                                                      |
| **Cycles**                       | `66,957,848`        | Total **CPU cycles** consumed during execution. Lower values generally mean faster execution.                                                        |
| **CPU Frequency**                | `3.022 GHz`         | Effective **CPU clock speed** during execution.                                                                                                      |
| **Instructions**                 | `21,933,617`        | Number of **CPU instructions** executed.                                                                                                             |
| **IPC (Instructions Per Cycle)** | `0.33`              | Efficiency metric: **higher IPC** means better execution per cycle. Low IPC may indicate cache misses, branch mispredictions, or memory bottlenecks. |
| **Branches**                     | `3,864,534`         | Total number of **branch instructions** executed (e.g., `if`, `for`, `while`). More branches may mean more **control flow changes**.                 |
| **Branch Misses**                | `18,917`            | The number of **mis-predicted branches**.                                                                                                            |
| **Branch Misprediction Rate**    | `0.49%`             | Percentage of branches that were **incorrectly predicted**. High values indicate poor branch prediction, leading to pipeline stalls.                 |

---

### **Time Breakdown**
| **Metric**       | **Value**             | **Description**                                                            |
|------------------|-----------------------|----------------------------------------------------------------------------|
| **Elapsed Time** | `0.022738693 seconds` | Total real-world **wall-clock time** from start to end.                    |
| **User Time**    | `0.016393000 seconds` | Time spent executing **actual code** (excluding kernel time).              |
| **System Time**  | `0.005786000 seconds` | Time spent in **kernel/system calls** (e.g., memory allocation, file I/O). |

---

## **How to Use This Reference**
1. **Compare Task Clock & Elapsed Time**  
   - If **Task Clock ≈ Elapsed Time**, your program ran efficiently with minimal interruptions.  
   - If **Task Clock > Elapsed Time**, your process was parallelized across multiple cores.  
   - If **Task Clock < Elapsed Time**, the process was waiting for I/O or system resources.

2. **Check IPC (Instructions Per Cycle)**
   - `IPC > 1.0` → Good CPU utilization (e.g., computational workloads).
   - `IPC < 1.0` → Potential **bottlenecks** (e.g., cache misses, branch mispredictions, memory stalls).

3. **Look at Branch Misses**
   - **High branch-miss rate (>2%)** → Poor branch prediction, try optimizing loops/conditionals.

4. **Analyze Context Switches & CPU Migrations**
   - **High values** indicate excessive OS scheduling, reducing performance.

5. **Monitor Page Faults**
   - **Frequent page faults** indicate memory inefficiencies (e.g., excessive allocations, poor locality).

---

## **Summary**
- **Task Clock ≈ Execution Time** → Efficient execution.
- **High IPC (>1.0)** → Good CPU efficiency.
- **Low Context Switches & Migrations** → Stable execution.
- **Low Branch Misses** → Good branch prediction.
- **Low Page Faults** → Efficient memory access.

This reference should help you interpret `perf stat` outputs consistently across **different programs**.

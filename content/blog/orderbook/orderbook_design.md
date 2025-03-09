---
title: "Order Book Design and Types"
date: 2025-02-06T23:30:00
author: "Pralay Patoria"
draft: false
---

# **Order Book Design: Market Orders, Limit Orders, and Advanced Order Types**  

## **1. Introduction**  
An order book is the core of any trading system, managing buy and sell orders and executing trades efficiently. This article explains:  
- How market and limit orders are handled.  
- The design and implementation of different order types.  
- Data structures, event-driven processing, and multi-threading where needed.  

## **2. Market Order vs Limit Order**  

### **Market Orders**  
- Executed immediately at the best available price.  
- Not stored in the order book.  
- Matched against existing limit orders.  

### **Limit Orders**  
- Placed with a specific price and stored in the order book.  
- Executed only when the market price meets the limit condition.  

### **Matching Process**  
- Market orders directly consume limit orders from the book.  
- A matching function iterates through limit orders, filling the market order until completed.  

## **3. Order Book Structure**  

### **Limit Order Book (LOB)**
- Stores limit orders sorted by price.  
- Typically implemented as a balanced BST (e.g., Red-Black Tree) or a skip list for O(log n) operations.  
- Orders at the same price are stored in a queue (FIFO).  

### **Market Order Handling**  
- No dedicated market order book.  
- Market orders are processed immediately using the limit order book’s best available prices.  

## **4. Advanced Limit Order Types**  

### **4.1 Immediate or Cancel (IOC)**
- Executes whatever is possible, cancels the rest.  
- Implemented by attempting matches and discarding unmatched quantities.  

### **4.2 Fill or Kill (FOK)**  
- Only executes if the full quantity can be matched immediately; otherwise, it is canceled.  
- Similar to IOC but requires full execution.  

### **4.3 Good Till Cancelled (GTC)**  
- Remains in the order book until executed or manually canceled.  
- Standard limit order logic, but stored beyond the current trading day.  

### **4.4 Good Till Time (GTT)**  
- Expires after a set time if not executed.  
- Requires additional scheduling for expiration.  

#### **GTT Implementation Details**  
- Stored like standard limit orders.  
- An expiration timestamp is tracked separately in a priority queue.  
- A scheduler runs periodically, checking and canceling expired orders.  

### **4.5 Stop-Limit Orders**  
- Two-phase order:  
  - **Stop price hit → Order activated.**  
  - **Limit order placed in LOB.**  
- Stop orders are stored in a hash table or multi-key BST indexed by price.  
- A market data listener checks price updates and activates stop orders when the stop price is reached.  

### **4.6 Iceberg Orders**  
- Large orders split into smaller visible portions.  
- Only a portion is placed in the limit order book at a time.  
- When the visible portion is fully executed, the next portion is scheduled.  

#### **Iceberg Order Handling**  
- Tracked separately using a hash table (Order ID → Remaining Quantity).  
- Event-driven approach: Order execution triggers replenishment of new visible portions.  

## **5. Event-Driven Design & Multi-Threading**  

### **5.1 Event Handling**  
- Every action (order placement, execution, cancellation) triggers an event.  
- Event-driven processing ensures efficient updates.  

### **5.2 Market Data Flow**  
- A market data listener continuously updates prices.  
- Stop-limit and GTT orders depend on real-time price changes.  

### **5.3 Parallel Processing**  
- Order matching runs in parallel for efficiency.  
- Market orders execute in a dedicated thread to avoid blocking.  
- Separate threads for price updates, order expiration, and trade execution.  

## **6. Conclusion**  
The order book follows a structured approach to ensure efficiency:  
- **Market orders** match against existing limit orders instantly.  
- **Limit orders** are stored, sorted, and matched when conditions meet.  
- **Complex orders** require additional tracking, scheduling, and event-based execution.  
- **Multi-threading** optimizes execution speed in real-world applications.  

This structured design ensures low-latency, high-efficiency order processing in trading systems.

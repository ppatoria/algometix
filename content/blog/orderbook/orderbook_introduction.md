---
title: "Design Considerations for a Order Book"
date: 2025-02-06T23:30:00
author: "Pralay Patoria"
draft: false
---


**Disclaimer:**  
This article presents a **preliminary attempt** at designing a basic order book using correct and optimal data structures provided by the C++ standard library. The focus is on selecting the right containers (`std::map`, `std::list`, and `std::unordered_map`) to meet the core requirements of an order book, such as sorted order maintenance, efficient insertions/deletions, stable iterators, and fast lookups.  

This discussion does **not** address advanced topics such as:  
- **Concurrency and thread safety** (e.g., locks, lock-free data structures, or atomic operations).  
- **Memory efficiency** (e.g., trade-offs between `std::list` and `std::vector`).  
- **Real-world edge cases** (e.g., market orders, partial fills, order amendments, or order book depth management).  
- **Performance benchmarks** (e.g., empirical measurements of insertion/deletion times or matching throughput).  
- **Scalability** (e.g., handling increasing order volumes or multiple symbols).  

These topics are critical for a production-grade trading system but are beyond the scope of this article. The goal here is to provide a foundational understanding of the design considerations and data structure choices for a basic order book implementation.

---

## 1. Requirements for Data Structures in an Order Book

**General Requirements:**
- **Sorted Order Maintenance:**  
  Orders must be grouped by price level, with buy orders sorted in descending order (to access the highest bid) and sell orders in ascending order (to access the lowest ask).

- **Efficient Insertion and Deletion:**  
  The system should support frequent updates—insertions, modifications, and cancellations—in logarithmic or constant time.

- **Stable Iterators and References:**  
  When mapping orders by their unique IDs, references (or iterators) to the orders must remain valid across most operations to avoid the risk of dangling pointers or invalid references.

- **Fast Lookup:**  
  The ability to quickly retrieve the best bid/ask and to locate orders by their ID is critical, especially under the high-frequency conditions of modern trading.

---

## 2. Using `std::map` for Price Levels

### **Requirements for Price Level Storage:**
- **Sorted Order:**  
  The container must automatically maintain orders in a sorted fashion—descending for bids and ascending for asks.
  
- **Efficient Operations:**  
  Insertion, deletion, and lookup operations should ideally run in O(log N) time.

- **Iterator Stability:**  
  Iterators should remain valid even as new price levels are added or removed (except when the element itself is removed).

### **Why `std::map`?**
- **Automatic Sorting:**  
  A `std::map` is typically implemented as a balanced binary search tree (e.g., Red-Black tree) and naturally maintains sorted order. For bids, the code uses `std::greater<Price>` to sort prices in descending order, while asks use the default `std::less<Price>` for ascending order.

- **Logarithmic Complexity:**  
  Insertions, deletions, and lookups are O(log N), which is acceptable given the volume of orders in a high-frequency trading system.

- **Stable Iterators:**  
  `std::map` guarantees that iterators remain valid even after insertions (and non-erasing modifications), ensuring that references in external mappings stay intact.

### **Why Not Alternatives?**
- **`std::unordered_map`:**  
  While offering average constant time lookup, it does not maintain any order, which is essential for quickly retrieving the best bid/ask.
  
- **Sorted Containers Using Vectors/Deques:**  
  Maintaining a sorted vector would require shifting elements during insertion/deletion and may invalidate iterators. Deques also risk iterator invalidation when elements are inserted or removed from the middle.

---

## 3. Using `std::list` for Orders at Each Price Level

### **Requirements for Order Storage Within a Price Level:**
- **Preserve Insertion Order (FIFO):**  
  Orders at the same price level should be executed in the order they were received.
  
- **Efficient Insertions/Deletions:**  
  The container should allow fast insertion at the end (for new orders) and removal from any position (for cancellations or partial executions).

- **Stable Iterators:**  
  Iterators must remain valid when new orders are added or others are removed, except for the removed element itself.

### **Why `std::list`?**
- **Constant-Time Insertions and Deletions:**  
  A `std::list` allows insertion at the back and deletion of elements in constant time without shifting other elements.

- **Iterator Stability:**  
  Unlike vectors or deques, `std::list` iterators remain valid even after insertions and deletions (aside from the element being erased). This is critical when maintaining an external mapping from order IDs to order locations.

### **Why Not Alternatives?**
- **`std::vector`:**  
  Although vectors provide good cache locality, inserting or deleting an order (especially in the middle) requires shifting elements, and reallocation can invalidate iterators.

- **`std::deque`:**  
  Deques may offer fast insertion at both ends, but they do not guarantee iterator stability if elements in the middle are altered. This makes them less suitable when external mappings depend on stable iterators.

- **`std::forward_list`:**  
  While it supports fast insertion and deletion, it only supports one-way traversal and lacks the flexibility of a doubly-linked list, which is often needed for more complex removal operations.

---

## 4. External Mapping Using `std::unordered_map`

### **Requirements for Order Lookup:**
- **Fast Retrieval by Order ID:**  
  Operations like cancellation or modification require quickly locating an order by its unique identifier.
  
- **Reference to Container Elements:**  
  The mapping must store a pair of iterators—one to the price level in the `std::map` and one to the order in the `std::list`.

### **Why `std::unordered_map`?**
- **Average Constant-Time Lookup:**  
  An `std::unordered_map` offers average O(1) lookup, which is essential for real-time order modifications.
  
- **Simplicity:**  
  Although rehashing can invalidate iterators, this mapping is solely used to quickly access order locations, and careful design prevents issues by immediately updating the map when orders are removed.

---

## 5. Code Considerations in Practice

### **Key Type Aliases (Class Members)**

In our implementation, we define the following type aliases as class members:

```cpp
using Price    = double;
using Symbol   = std::string;
using OrderID  = int;
using Quantity = size_t;

enum class Side { Buy = 1, Sell = 2 };

struct Order {
  OrderID  orderID;
  Price    price;
  Quantity quantity;
  Side     side;
  Symbol   symbol;
};

class OrderBook {
private:
  // Orders at a specific price level
  using Orders = std::list<Order>;

  // PriceLevel: map from Price to Orders, with a custom comparator for sorting.
  template <typename Comparator>
  using PriceLevel = std::map<Price, Orders, Comparator>;

  // Aliases for iterators and order locations
  using PriceLevelIterator = std::map<Price, Orders>::iterator;
  using OrderLocation      = std::pair<PriceLevelIterator, Orders::iterator>;
  using OrderMap           = std::unordered_map<OrderID, OrderLocation>;

  // Containers for buy and sell orders.
  // Bids: sorted in descending order (highest price first)
  PriceLevel<std::greater<Price>> bids;
  // Asks: sorted in ascending order (lowest price first)
  PriceLevel<std::less<Price>>    asks;

  // Global mapping from order ID to its location.
  OrderMap orderMap;

  // Templated helper function for insertion.
  template <typename T>
  void insert(T &priceLevels, const Order &order);

public:
  void insert(const Order &order);
  void cancel(const Order &order);
  void modify(const Order &order);
  void match();
};

using OrderBooks = std::unordered_map<Symbol, OrderBook>;
```

---

## 6. Conclusion

This article provides a foundational understanding of the design considerations and data structure choices for implementing a basic order book in C++. By leveraging `std::map`, `std::list`, and `std::unordered_map`, the implementation meets the core requirements of sorted order maintenance, efficient operations, stable iterators, and fast lookups. However, this is only a **preliminary attempt**, and a production-grade system would need to address advanced topics such as concurrency, memory efficiency, real-world edge cases, and scalability. These enhancements are beyond the scope of this discussion but are critical for building a robust and high-performance trading system.

---

This revised version ensures that readers understand the scope and limitations of the article while still providing valuable insights into the foundational design of an order book.

---
title: "Implementation of a Order Book"
date: 2025-02-06T23:30:00
author: "Pralay Patoria"
draft: false
---

In the first article, we discussed the **design considerations** and **data structure choices** for implementing a basic order book in C++. In this second article, we will dive into the **detailed implementation** of the order book, explaining the flow, scenarios, and minute design decisions. We will walk through the code step by step, highlighting why certain functions (e.g., `try_emplace`) are used, how orders are cleaned up, and how the matching algorithm works.

---

## 1. Overview of the Implementation

The order book implementation consists of the following key components:
1. **Order Struct**: Represents a single order with fields like `orderID`, `price`, `quantity`, `side`, and `symbol`.
2. **OrderBook Class**: Manages two main containers:
   - **Bids**: A `std::map` sorted in descending order of prices.
   - **Asks**: A `std::map` sorted in ascending order of prices.
3. **OrderMap**: A `std::unordered_map` that maps `orderID` to the location of the order in the order book.
4. **Core Functions**: `insert`, `cancel`, `modify`, and `match`.

---

## 2. Key Design Decisions and Code Walkthrough

### **2.1 Order Struct**

```cpp
struct Order {
  OrderID  orderID;    // Unique identifier for the order
  Price    price;      // Order price
  Quantity quantity;   // Number of units to trade
  Side     side;       // Order side: Buy or Sell
  Symbol   symbol;     // Instrument symbol (e.g., "AAPL")
};
```

**Why This Design?**
- The `Order` struct encapsulates all the necessary fields for an order.
- The `side` field (of type `enum class Side`) ensures type safety and readability.
- The `symbol` field allows the order book to support multiple instruments (though this is not fully utilized in the current implementation).

---

### **2.2 OrderBook Class**

#### **2.2.1 Data Members**

```cpp
class OrderBook {
private:
  using Orders = std::list<Order>;  // List of orders at a specific price level

  template <typename Comparator>
  using PriceLevel = std::map<Price, Orders, Comparator>;

  using PriceLevelIterator = std::map<Price, Orders>::iterator;
  using OrderLocation      = std::pair<PriceLevelIterator, Orders::iterator>;
  using OrderMap           = std::unordered_map<OrderID, OrderLocation>;

  PriceLevel<std::greater<Price>> bids;  // Buy orders: sorted descending
  PriceLevel<std::less<Price>>    asks;  // Sell orders: sorted ascending

  OrderMap orderMap;  // Mapping from OrderID to order location in the book
};
```

**Why These Data Structures?**
- **`std::map` for Price Levels**:  
  - `bids` and `asks` are `std::map` containers with custom comparators (`std::greater` for bids and `std::less` for asks).  
  - This ensures that bids are sorted in descending order (highest price first) and asks in ascending order (lowest price first).  
  - `std::map` provides O(log N) insertion, deletion, and lookup, which is efficient for price levels.

- **`std::list` for Orders**:  
  - Orders at each price level are stored in a `std::list`.  
  - `std::list` provides O(1) insertion and deletion, and its iterators remain valid unless the element itself is erased. This is critical for maintaining stable references in `orderMap`.

- **`std::unordered_map` for Order Lookup**:  
  - `orderMap` maps `orderID` to a pair of iterators (`PriceLevelIterator` and `Orders::iterator`).  
  - This allows O(1) lookup for order modifications and cancellations.
  - **Note**: The `OrderMap` stores a pair of iterators (`PriceLevelIterator` and `Orders::iterator`) instead of just the order iterator. This allows direct access to the price level for efficient cleanup and avoids redundant O(log N) lookups in the `std::map`, ensuring robust and consistent behavior.


---

This version is brief and to the point while still conveying the key rationale.
---

### **2.3 Insert Function**

#### **2.3.1 Templated Insert Helper**

```cpp
template <typename T>
void OrderBook::insert(T &priceLevels, const Order &order) {
  if (orderMap.count(order.orderID))
    throw std::runtime_error("Order already exists.");

  auto [priceLevelIter, inserted] =
      priceLevels.try_emplace(order.price, Orders{});

  priceLevelIter->second.push_back(order);
  auto orderIter = std::prev(priceLevelIter->second.end());
  orderMap[order.orderID] = std::make_pair(priceLevelIter, orderIter);
}
```

**Why `try_emplace`?**
- `try_emplace` is used to insert a new price level if it does not already exist.  
- It avoids unnecessary construction of an `Orders` object if the price level already exists, improving performance.  
- It returns an iterator to the price level and a boolean indicating whether the insertion occurred.

**Why `std::prev`?**
- After appending the order to the `std::list`, `std::prev` is used to obtain an iterator to the newly inserted order.  
- This is necessary because `push_back` does not return an iterator.

**Why Update `orderMap`?**
- The `orderMap` is updated with the location of the new order (price level iterator and order iterator).  
- This allows O(1) lookup for future modifications or cancellations.

#### **2.3.2 Public Insert Function**

```cpp
void OrderBook::insert(const Order &order) {
  switch (order.side) {
    case Side::Buy:
      insert(bids, order);
      break;
    case Side::Sell:
      insert(asks, order);
      break;
    default:
      throw std::runtime_error("Invalid order side.");
  }
}
```

**Why This Design?**
- The public `insert` function delegates to the templated helper based on the order side.  
- This avoids code duplication and ensures consistent behavior for bids and asks.

---

### **2.4 Cancel Function**

```cpp
void OrderBook::cancel(const Order &order) {
  auto iter = orderMap.find(order.orderID);
  if (iter == orderMap.end())
    throw std::runtime_error("Order does not exist in the OrderBook.");

  auto [priceLevelIter, orderIter] = iter->second;
  auto &orderList = priceLevelIter->second;

  orderList.erase(orderIter);

  if (orderList.empty()) {
    if (order.side == Side::Buy) {
      bids.erase(priceLevelIter);
    } else if (order.side == Side::Sell) {
      asks.erase(priceLevelIter);
    }
  }

  orderMap.erase(order.orderID);
}
```

**Why Clean Up Empty Price Levels?**
- If the `orderList` becomes empty after cancellation, the price level is removed from the `std::map`.  
- This prevents the accumulation of empty price levels, which would waste memory and slow down operations.

**Why Update `orderMap`?**
- The `orderMap` is updated to remove the canceled order, ensuring that the mapping remains consistent.

---

### **2.5 Modify Function**

```cpp
void OrderBook::modify(const Order &order) {
  auto iter = orderMap.find(order.orderID);
  if (iter == orderMap.end())
    throw std::runtime_error("Order not found in the orderbook.");

  auto &orderToModify = *(iter->second.second);
  if (orderToModify.orderID != order.orderID ||
      orderToModify.side    != order.side    ||
      orderToModify.symbol  != order.symbol)
    throw std::runtime_error("Order to modify is different than the incoming order");

  if (orderToModify.price != order.price) {
    cancel(order);
    insert(order);
    return;
  }

  orderToModify.quantity = order.quantity;
}
```

**Why Cancel and Reinsert for Price Changes?**
- If the price changes, the order must be moved to a new price level.  
- This is achieved by canceling the existing order and reinserting it with the new price.  
- This ensures that the order book remains sorted and consistent.

**Why Check Order Identity?**
- The function verifies that the `orderID`, `side`, and `symbol` of the incoming order match the stored order.  
- This prevents accidental modifications of the wrong order.

---

### **2.6 Match Function**

```cpp
void OrderBook::match() {
  while (!bids.empty() && !asks.empty()) {
    auto bidsIter = bids.begin();
    auto asksIter = asks.begin();
    auto &[bestBidPrice, bestBidOrders] = *bidsIter;
    auto &[bestAskPrice, bestAskOrders] = *asksIter;

    if (bestBidPrice < bestAskPrice)
      break;

    auto bidOrderIter = bestBidOrders.begin();
    auto askOrderIter = bestAskOrders.begin();

    while (bidOrderIter != bestBidOrders.end() &&
           askOrderIter != bestAskOrders.end()) {
      auto maxQuantityMatched =
          std::min(bidOrderIter->quantity, askOrderIter->quantity);

      std::cout << "Executing matched order: "
                << " Bid OrderID    : " << bidOrderIter->orderID << "\n"
                << " Ask OrderID    : " << askOrderIter->orderID << "\n"
                << " Symbol         : " << askOrderIter->symbol << "\n"
                << " Trade Price    : " << bestAskPrice << "\n"
                << " Order Quantity : " << maxQuantityMatched 
                << std::endl;

      bidOrderIter->quantity -= maxQuantityMatched;
      askOrderIter->quantity -= maxQuantityMatched;

      auto cleanupAndIterate = [&](auto &orderIter, auto &orderList) {
        if (orderIter->quantity == 0) {
          auto orderID = orderIter->orderID;
          orderIter = orderList.erase(orderIter);//Erase returns next valid iterator.
          orderMap.erase(orderID);
        } else {
          ++orderIter;
        }
      };

      cleanupAndIterate(bidOrderIter, bestBidOrders);
      cleanupAndIterate(askOrderIter, bestAskOrders);
    }

    if (bidsIter->second.empty())
      bids.erase(bidsIter);
    if (asksIter->second.empty())
      asks.erase(asksIter);
  }
}
```

**Why Use a Lambda for Cleanup?**
The `cleanupAndIterate` lambda is used to handle the cleanup of fully matched orders and advance iterators for partially matched orders. Here’s why this approach is used:

1. **Implicit and Explicit Iterator Advancement**:  
   - When an order is fully matched (quantity becomes zero), it is erased from the `std::list`. The `std::list::erase` method **implicitly advances the iterator** to the next element, so no explicit increment is needed in this case.  
   - For partially matched orders (quantity > 0), the iterator is **explicitly incremented** to move to the next order in the list.  
   - This dual behavior (implicit and explicit iteration** is handled seamlessly by the lambda.
Here’s the concise version:

**Note** The lambda is placed close to the matching logic for readability and to avoid duplication, but a private method is preferable for modularity, reusability, and testing in production.

**Why Remove Empty Price Levels?**
- After processing a price level, if it becomes empty, it is removed from the `std::map`.  
- This ensures that the order book remains efficient and free of unnecessary price levels.


#### Visual Walkthrough: Order Matching Example

Assume the order book state is as follows before matching:

**Initial Order Book:**

```
Bids:
  101.0  --> [Order 1: Qty = 80]

Asks:
   99.0  --> [Order 3: Qty = 70]
  101.0  --> [Order 5: Qty = 20]
  102.0  --> [Order 4: Qty = 30]
```

*Note: Order 1 is a buy order (modified to 80), Order 3, 5, and 4 are sell orders.*

---

**Step 1: Matching Best Bid vs. Best Ask**

- **Best Bid:** Order 1 at 101.0 (Qty = 80)  
- **Best Ask:** Order 3 at 99.0 (Qty = 70)

Since **101.0 ≥ 99.0**, a match occurs.

- **Matched Quantity:** min(80, 70) = 70  
- **Updates:**  
  - Order 1: Qty decreases from 80 to 10  
  - Order 3: Qty decreases from 70 to 0 (fully filled → remove)

**Order Book After Step 1:**

```
Bids:
  101.0  --> [Order 1: Qty = 10]

Asks:
  101.0  --> [Order 5: Qty = 20]
  102.0  --> [Order 4: Qty = 30]
```

---

**Step 2: Continue Matching with Updated Orders**

- **Best Bid:** Order 1 at 101.0 (Qty = 10)  
- **Best Ask:** Order 5 at 101.0 (Qty = 20)

Since **101.0 ≥ 101.0**, a match occurs.

- **Matched Quantity:** min(10, 20) = 10  
- **Updates:**  
  - Order 1: Qty decreases from 10 to 0 (fully filled → remove)  
  - Order 5: Qty decreases from 20 to 10

**Order Book After Step 2:**

```
Bids:
  (empty)

Asks:
  101.0  --> [Order 5: Qty = 10]
  102.0  --> [Order 4: Qty = 30]
```

---

**Step 3: Matching Stops**

- With **no bids remaining**, matching stops.
- Remaining sell orders continue to reside in the book until new buy orders arrive.

---

##### Summary Diagram

```plaintext
Initial Order Book:
--------------------
Bids:
  101.0: [Order 1, Qty=80]

Asks:
   99.0: [Order 3, Qty=70]
  101.0: [Order 5, Qty=20]
  102.0: [Order 4, Qty=30]
--------------------

Step 1:
Match Order 1 (101.0, Qty=80) vs. Order 3 (99.0, Qty=70)
-> Matched Qty = 70
Order 1: 80 -> 10; Order 3: 70 -> 0 (remove Order 3)
--------------------
Bids:
  101.0: [Order 1, Qty=10]

Asks:
  101.0: [Order 5, Qty=20]
  102.0: [Order 4, Qty=30]
--------------------

Step 2:
Match Order 1 (101.0, Qty=10) vs. Order 5 (101.0, Qty=20)
-> Matched Qty = 10
Order 1: 10 -> 0 (remove Order 1); Order 5: 20 -> 10
--------------------
Bids:
  (empty)

Asks:
  101.0: [Order 5, Qty=10]
  102.0: [Order 4, Qty=30]
--------------------

Step 3:
No bids available; matching stops.
```
---

## 3. Conclusion

This article provided a **detailed walkthrough** of the order book implementation, explaining the flow, design decisions, and minute details. By leveraging `std::map`, `std::list`, and `std::unordered_map`, the implementation achieves efficient operations, stable iterators, and fast lookups. However, this is only a **basic implementation**, and a production-grade system would need to address advanced topics such as concurrency, memory efficiency, and scalability. These enhancements will be explored in future articles in this series.

---

This article complements the first one by diving deeper into the implementation, making it a valuable resource for understanding the intricacies of designing an order book in C++.

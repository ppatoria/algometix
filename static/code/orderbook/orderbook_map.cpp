/**
 * Order Book Design Requirements:
 * --------------------------------------------------------
 * 1. Data Storage:
 *    - Support multiple instruments/symbols, each with its own order book.
 *    - Group orders by price level, ensuring that within each level orders 
 *      are maintained in FIFO order.
 *    - Each order should have a unique identifier along with fields such as 
 *      price, quantity, and side (buy or sell).
 *
 * 2. Data Retrieval & Access:
 *    - Enable efficient lookup by price and by order ID.
 *    - Best bid (highest buy price) and best ask (lowest sell price) must be 
 *      retrievable in constant or logarithmic time.
 *    - Maintain price levels in sorted order:
 *         * Buy orders: sorted in descending order.
 *         * Sell orders: sorted in ascending order.
 *
 * 3. Efficiency Considerations:
 *    - Insertion of orders should be fast (preferably O(1) or O(log N)).
 *    - Deletion and modification (cancellation, amendment) should incur minimal
 *      overhead given their high frequency.
 *    - Order matching must quickly traverse the best prices, supporting 
 *      high-frequency trading requirements.
 *    - Overall throughput and latency must be optimized, with low memory 
 *      overhead and minimal copying or reallocation.
 *
 * 4. Mapping & Reference Stability:
 *    - The design should allow rapid access to an order by its ID, to support 
 *      fast cancellations and modifications.
 *    - References (iterators or indices) to orders must remain valid across most
 *      operations (insertion, updates), ensuring that external mappings do not
 *      become inconsistent.
 *
 * 5. Concurrency & Robustness:
 *    - The order book must be designed for concurrent access, ensuring 
 *      thread-safety in a multi-threaded environment.
 *    - Operations should be robust under heavy load, guaranteeing data 
 *      integrity and consistency at all times.
 *
 * Overall, the design must ensure:
 * - Low latency in order insertion, deletion, and matching.
 * - Efficient retrieval of orders and price levels.
 * - Stable external references for fast, reliable updates.
 * - Scalability to handle high order volumes typical in modern markets.
 */

/**
 * Important Design Consideration: Mapping IDs to Iterators
 * --------------------------------------------------------
 * When maintaining an external mapping (e.g.,
 * std::unordered_map<OrderID, IteratorType>) where the iterator points to an
 * element inside a container, ensure that the container guarantees stable
 * iterators across insertions and deletions.
 *
 * SAFE CONTAINERS:
 * - std::list and std::forward_list:
 *   Iterators remain valid unless the referenced element is erased.
 * - std::set and std::map:
 *   Iterators remain valid unless the referenced element is erased.
 *
 * UNSAFE CONTAINERS:
 * - std::vector:
 *   Iterators are invalidated when resizing occurs or when elements are erased.
 * - std::deque:
 *   Erasing from the front or middle invalidates iterators, making stored
 *   references unreliable.
 * - std::unordered_map:
 *   Rehashing invalidates all iterators, though element insertions and deletions
 *   (without rehashing) do not.
 *
 * DESIGN WARNING:
 * Storing iterators in an external map can lead to dangling iterators if
 * elements are removed or the container resizes, resulting in undefined
 * behavior.
 *
 * RECOMMENDED APPROACH:
 * - Prefer std::list or std::forward_list when maintaining a mapping from IDs
 *   to elements.
 * - If using std::vector or std::deque, consider storing indices instead of
 *   iterators, though indices must be updated on removals.
 */

#include <list>
#include <map>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <algorithm>
#include <iostream>

using Price    = double;
using Symbol   = std::string;
using OrderID  = int;
using Quantity = size_t;

enum class Side { Buy = 1, Sell = 2 };

/*
 ***************************************
 * Order
 ***************************************
 *
 * Represents a single order in the order book.
 * Each order has a unique OrderID, a price, a quantity,
 * a side (Buy or Sell), and a symbol (instrument).
 */
struct Order {
  OrderID  orderID;    /* Unique identifier for the order */
  Price    price;      /* Order price */
  Quantity quantity;   /* Number of units to trade */
  Side     side;       /* Order side: Buy or Sell */
  Symbol   symbol;     /* Instrument symbol (e.g., "AAPL") */
};

class OrderMatching;  /* Forward declaration for potential matching logic */

/*
 ***************************************
 * OrderBook: Asks and Bids
 ***************************************
 *
 * Maintains separate collections for Buy (bids) and Sell (asks)
 * orders. Orders are grouped by price level:
 *   - Bids are stored in descending order (highest price first).
 *   - Asks are stored in ascending order (lowest price first).
 *
 * An external map (orderMap) maintains a mapping from OrderID
 * to its location (price level iterator and order iterator) for
 * fast lookup, modification, or cancellation.
 */
class OrderBook {
private:
  using Orders = std::list<Order>;  /* List of orders at a specific price level */

  /*
   * PriceLevel is a map that associates a price with a list of orders.
   * The Comparator defines the sorting order:
   *   - For bids: std::greater<Price> (descending)
   *   - For asks: std::less<Price> (ascending)
   */
  template <typename Comparator>
  using PriceLevel = std::map<Price, Orders, Comparator>;

  using PriceLevelIterator = std::map<Price, Orders>::iterator;
  using OrderLocation      = std::pair<PriceLevelIterator, Orders::iterator>;
  using OrderMap           = std::unordered_map<OrderID, OrderLocation>;

  PriceLevel<std::greater<Price>> bids;  /* Buy orders: sorted descending */
  PriceLevel<std::less<Price>>    asks;   /* Sell orders: sorted ascending */

  OrderMap orderMap;  /* Mapping from OrderID to order location in the book */

  /*
   * Templated insert function to add an order to the given priceLevels
   * container (either bids or asks). This function handles:
   *   - Checking for duplicate orders.
   *   - Inserting a new price level if needed.
   *   - Appending the order to the list at that price level.
   *   - Updating the orderMap with the order's location.
   */
  template <typename T>
  void insert(T &priceLevels, const Order &order);

public:
  /*
   * Inserts an order into the OrderBook.
   * Determines the side (Buy or Sell) and delegates insertion to the
   * appropriate container (bids or asks).
   */
  void insert(const Order &order);

  /*
   * Cancels an order from the OrderBook.
   * The order is located via orderMap, removed from its price level,
   * and the orderMap is updated accordingly.
   */
  void cancel(const Order &order);

  /*
   * Modifies an existing order.
   * If the order's price has changed, it is cancelled and reinserted;
   * otherwise, only the quantity is updated.
   */
  void modify(const Order &order);

  /*
   * Matches orders in the OrderBook.
   * Continuously processes the best bid and ask price levels and attempts
   * to match orders until no further matching is possible.
   */
  void match();
};

/*
 ***************************************
 * OrderBooks: A collection of OrderBooks
 ***************************************
 *
 * Maps a Symbol to its corresponding OrderBook.
 */
using OrderBooks = std::unordered_map<Symbol, OrderBook>;

/*
 ***************************************
 * OrderBook: Implementation
 ***************************************
 */

/*
 * Templated insert function for OrderBook.
 * Inserts an order into the provided priceLevels container.
 */
template <typename T>
void OrderBook::insert(T &priceLevels, const Order &order) {

  /* 
   * Check for duplicate OrderID in orderMap. If the order already exists,
   * throw an exception to prevent duplicate entries.
   */
  if (orderMap.count(order.orderID))
    throw std::runtime_error("Order already exists.");

  /* 
   * Attempt to insert a new price level for the order's price using
   * try_emplace. If the price level does not exist, this creates a new
   * entry with an empty list (Orders{}).
   * 
   * try_emplace returns a pair:
   *   - priceLevelIter: Iterator to the price level for order.price.
   *   - inserted: Boolean indicating whether a new price level was inserted.
   */
  auto [priceLevelIter, inserted] =
      priceLevels.try_emplace(order.price, Orders{});

  /* 
   * Append the order to the Orders list at the determined price level.
   * The push_back operation adds the order to the end of the list.
   */
  priceLevelIter->second.push_back(order);

  /* 
   * Obtain an iterator to the newly inserted order.
   * Since push_back appends to the end, std::prev on end() gives an iterator
   * to the last element (i.e., the new order).
   */
  auto orderIter = std::prev(priceLevelIter->second.end());

  /* 
   * Update the global orderMap with the location of the new order.
   * The location is represented as a pair of iterators:
   *   - The first iterator points to the price level.
   *   - The second iterator points to the order within the Orders list.
   */
  orderMap[order.orderID] = std::make_pair(priceLevelIter, orderIter);
}

/*
 * Inserts an order into the OrderBook.
 * Determines the order side and calls the templated insert function with
 * the appropriate container.
 */
void OrderBook::insert(const Order &order) {

  switch (order.side) {
  case Side::Buy:
    /* 
     * For Buy orders, insert into the bids container.
     * Bids are sorted in descending order, ensuring that the highest bid
     * is always at the beginning.
     */
    insert(bids, order);
    break;
  case Side::Sell:
    /* 
     * For Sell orders, insert into the asks container.
     * Asks are sorted in ascending order, so the lowest ask is at the front.
     */
    insert(asks, order);
    break;
  default:
    /* 
     * If the order side is invalid, throw an exception.
     */
    throw std::runtime_error("Invalid order side.");
  }
}

/*
 * Cancels an order from the OrderBook.
 */
void OrderBook::cancel(const Order &order) {
  /* 
   * Locate the order in the global orderMap using its OrderID.
   * The orderMap stores a pair:
   *   - First element: Iterator to the price level in bids or asks.
   *   - Second element: Iterator to the specific order in the Orders list.
   */
  auto iter = orderMap.find(order.orderID);
  if (iter == orderMap.end()) {
    /* 
     * If the order is not found, throw an exception to indicate that the
     * order does not exist in the OrderBook.
     */
    throw std::runtime_error("Order does not exists in the OrderBook.");
  }

  /* 
   * Decompose the stored pair to obtain:
   *   - priceLevelIter: Iterator to the price level.
   *   - orderIter: Iterator to the order within the Orders list.
   */
  auto [priceLevelIter, orderIter] = iter->second;

  /* 
   * Get a reference to the list of orders at the identified price level.
   */
  auto &orderList = priceLevelIter->second;

  /* 
   * Erase the order from the order list using the order iterator.
   * This effectively removes the order from its price level.
   */
  orderList.erase(orderIter);

  /* 
   * If the order list for the price level becomes empty after removal,
   * remove the entire price level from the OrderBook.
   * This prevents empty price levels from accumulating.
   */
  if (orderList.empty()) {
    if (order.side == Side::Buy) {
      /* Remove price level from bids for Buy orders */
      bids.erase(priceLevelIter);
    } else if (order.side == Side::Sell) {
      /* Remove price level from asks for Sell orders */
      asks.erase(priceLevelIter);
    }
  }

  /* 
   * Finally, erase the order from the global orderMap so that it is fully
   * removed from the OrderBook.
   */
  orderMap.erase(order.orderID);
}

/*
 * Modifies an existing order in the OrderBook.
 */
void OrderBook::modify(const Order &order) {
  /* 
   * Locate the order in the orderMap using its OrderID.
   * The orderMap maps an OrderID to a pair of iterators (price level and 
   * order within that level).
   */
  auto iter = orderMap.find(order.orderID);

  /* 
   * If the order is not found, throw an error. This prevents modifications
   * to non-existent orders.
   */
  if (iter == orderMap.end())
    throw std::runtime_error("Order not found in the orderbook.");

  /* 
   * Dereference the second iterator to obtain a reference to the stored order.
   */
  auto &orderToModify = *(iter->second.second);

  /* 
   * Verify that the stored order's identifying fields (OrderID, side, symbol)
   * match those of the incoming order. This check prevents accidental
   * modification of the wrong order.
   *
   * Corner Case: A mismatch indicates a logical error or misrouted update,
   * so an exception is thrown.
   */
  if (orderToModify.orderID != order.orderID ||
      orderToModify.side    != order.side    ||
      orderToModify.symbol  != order.symbol) {
    throw std::runtime_error(
        "Order to modify is different than the incoming order");
  }

  /* 
   * Check if the price has changed. Since the price determines the order's
   * position in the sorted order book, a change in price requires removal
   * from the current price level and reinsertion into the correct level.
   *
   * Issue Resolved: Directly updating the price could leave the order in an
   * incorrect position in the sorted map.
   */
  if (orderToModify.price != order.price) {
    cancel(order);
    insert(order);
    return;
  }

  /* 
   * If the price remains unchanged, simply update the order's quantity.
   * This handles partial cancellations or amendments without altering order
   * positioning.
   */
  orderToModify.quantity = order.quantity;
}

/*
 * Matches orders in the OrderBook.
 *
 * The matching algorithm continuously attempts to match the best bid and 
 * best ask price levels. It traverses the order lists at these price levels,
 * matching orders based on the minimum available quantity.
 *
 * Flow:
 *   1. While both bids and asks exist, retrieve the best bid and ask price
 *      levels.
 *   2. If the best bid price is lower than the best ask price, matching stops.
 *   3. Otherwise, iterate over the orders in the best bid and ask lists.
 *   4. For each pair, compute the maximum quantity that can be matched.
 *   5. Reduce the quantities of the matched orders by the matched amount.
 *   6. Use a lambda function to:
 *        - Remove fully matched orders (quantity becomes zero), updating
 *          the orderMap accordingly.
 *        - Advance the iterator if the order is only partially matched.
 *   7. After processing the current price level, if it becomes empty,
 *      remove the entire level from the OrderBook.
 *
 * Corner Cases & Issues Prevented:
 *   - Ensures that matching stops when there is no cross (bid < ask).
 *   - Prevents dangling iterators by updating iterators using the return
 *     value of erase().
 *   - Removes empty price levels to maintain a clean order book.
 */
void OrderBook::match() {

  /* 
   * Outer loop: Continue matching as long as there are orders on both sides.
   * This ensures that matching only occurs when both bids and asks exist.
   */
  while (!bids.empty() && !asks.empty()) {

    /* 
     * Retrieve the best bid and ask price levels.
     * - bids.begin() returns the highest bid because bids are sorted in 
     *   descending order.
     * - asks.begin() returns the lowest ask because asks are sorted in 
     *   ascending order.
     *
     * Structured bindings decompose the map entries into:
     *   - bestBidPrice and bestBidOrders for bids.
     *   - bestAskPrice and bestAskOrders for asks.
     */
    auto bidsIter = bids.begin();
    auto asksIter = asks.begin();
    auto &[bestBidPrice, bestBidOrders] = *bidsIter;
    auto &[bestAskPrice, bestAskOrders] = *asksIter;

    /* 
     * Price Check: If the best bid price is less than the best ask price,
     * no trade can occur. Exit the matching loop.
     */
    if (bestBidPrice < bestAskPrice)
      break;

    /* 
     * Initialize iterators for the orders at the best bid and ask price levels.
     */
    auto bidOrderIter = bestBidOrders.begin();
    auto askOrderIter = bestAskOrders.begin();

    /* 
     * Inner loop: Process matching for orders at the current price level.
     * Continues as long as there are orders remaining in both the bid and
     * ask lists.
     */
    while (bidOrderIter != bestBidOrders.end() &&
           askOrderIter != bestAskOrders.end()) {

      /* 
       * Calculate the maximum quantity that can be matched between the
       * current bid and ask orders.
       * This is the minimum of the quantities of the two orders.
       */
      auto maxQuantityMatched =
          std::min(bidOrderIter->quantity, askOrderIter->quantity);

      /* 
       * Output details of the executed match.
       * This includes:
       *   - Bid OrderID
       *   - Ask OrderID
       *   - Symbol (should be the same for both orders)
       *   - Trade Price (using bestAskPrice; adjust based on pricing rules)
       *   - Matched Order Quantity
       */
      std::cout << "Executing matched order: "
                << " Bid OrderID    : " << bidOrderIter->orderID << "\n"
                << " Ask OrderID    : " << askOrderIter->orderID << "\n"
                << " Symbol         : " << askOrderIter->symbol << "\n"
                << " Trade Price    : " << bestAskPrice << "\n"
                << " Order Quantity : " << maxQuantityMatched 
                << std::endl;

      /* 
       * Reduce the quantities of the current bid and ask orders by the
       * matched quantity.
       */
      bidOrderIter->quantity -= maxQuantityMatched;
      askOrderIter->quantity -= maxQuantityMatched;

      /* 
       * Lambda function to clean up orders after matching:
       *   - If an order's quantity becomes zero, it is fully matched.
       *     The order is erased from the order list, and its entry in the 
       *     orderMap is removed.
       *   - If the order is only partially matched, simply advance the iterator.
       *
       * The lambda takes the iterator and the corresponding order list by
       * reference so that updates persist outside the lambda.
       */
      auto cleanupAndIterate = [&](auto &orderIter, auto &orderList) {
        if (orderIter->quantity == 0) {
          /* Capture the orderID before erasing to update the orderMap */
          auto orderID = orderIter->orderID;
          /* Erase returns the next valid iterator, updating orderIter */
          orderIter = orderList.erase(orderIter);
          /* Remove the order from the global orderMap */
          orderMap.erase(orderID);
        } else {
          /* Advance the iterator if the order is not fully matched */
          ++orderIter;
        }
      };

      /* 
       * Clean up the current bid and ask orders using the lambda.
       * This ensures that fully matched orders are removed and the iterators
       * are correctly advanced.
       */
      cleanupAndIterate(bidOrderIter, bestBidOrders);
      cleanupAndIterate(askOrderIter, bestAskOrders);
    }

    /* 
     * After processing orders at the current price level, check if the
     * price level is empty.
     * If the order list for the best bid or ask price is empty, remove the
     * entire price level from the OrderBook.
     */
    if (bidsIter->second.empty())
      bids.erase(bidsIter);
    if (asksIter->second.empty())
      asks.erase(asksIter);
  }
}

/*
 ***************************************
 * Main
 ***************************************
 *
 * Program entry point.
 * In this example, main() does nothing; it is a placeholder for
 * integration with further order book operations.
 */


/*
 * Main Function:
 *
 * This function demonstrates usage of the OrderBook by performing a series
 * of operations including insertions, cancellations, modifications, and 
 * order matching. It simulates a simple trading scenario for the symbol 
 * "AAPL".
 */
int main() {
  /* Create an instance of OrderBook for "AAPL". */
  OrderBook ob;

  /* 
   * Insert several orders into the order book.
   * For demonstration, we insert a mix of buy and sell orders with varying 
   * prices and quantities.
   */

  /* Buy Order: OrderID 1, Price 101.0, Quantity 100, Side Buy */
  Order order1 {1, 101.0, 100, Side::Buy, "AAPL"};
  ob.insert(order1);

  /* Buy Order: OrderID 2, Price 100.0, Quantity 50, Side Buy */
  Order order2 {2, 100.0, 50, Side::Buy, "AAPL"};
  ob.insert(order2);

  /* Sell Order: OrderID 3, Price 99.0, Quantity 70, Side Sell */
  Order order3 {3, 99.0, 70, Side::Sell, "AAPL"};
  ob.insert(order3);

  /* Sell Order: OrderID 4, Price 102.0, Quantity 30, Side Sell */
  Order order4 {4, 102.0, 30, Side::Sell, "AAPL"};
  ob.insert(order4);

  /* Sell Order: OrderID 5, Price 101.0, Quantity 20, Side Sell */
  Order order5 {5, 101.0, 20, Side::Sell, "AAPL"};
  ob.insert(order5);

  /* 
   * Cancel an order.
   * For example, cancel the buy order with OrderID 2.
   * This demonstrates how an order can be removed from the book.
   */
  ob.cancel(order2);

  /* 
   * Modify an existing order.
   * Modify OrderID 1 by changing its quantity from 100 to 80.
   * Since the price remains unchanged, the order remains in the same price 
   * level.
   */
  Order modOrder1 = order1;  // Create a copy of order1 for modification.
  modOrder1.quantity = 80;
  ob.modify(modOrder1);

  /* 
   * Perform order matching.
   * The match() function processes the best bid and ask price levels and
   * attempts to match orders based on available quantities.
   *
   * In our scenario:
   *   - The best bid is from OrderID 1 at price 101.0.
   *   - The best ask is from OrderID 3 at price 99.0.
   * Since 101.0 >= 99.0, matching will occur.
   */
  ob.match();

  return 0;
}


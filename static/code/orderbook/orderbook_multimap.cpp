#include <algorithm>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <unordered_map>

enum class Side { Buy, Sell };

struct order {
  int id;
  int quantity;
  double price;
  std::string symbol;
  Side side;
};

class orderbook {

  using PriceLevel = std::multimap<double, order>;
  PriceLevel bids;
  PriceLevel asks;

  std::unordered_map<int, PriceLevel::iterator> orderMap;

public:
  void insert(const order& ord){
    auto iter = orderMap.find(ord.id);
    if(iter != orderMap.end()){
      throw std::runtime_error("Order already exists.");
    }
    auto& priceLevels = (ord.side == Side::Buy)
                      ? bids
                      : asks;
    auto priceLevelIter = priceLevels.insert({ord.price, ord});
    orderMap[ord.id] = priceLevelIter;
  }

  void cancel(const order& ord){
    auto iter = orderMap.find(ord.id);
    if(iter == orderMap.end()){
      throw std::runtime_error("Order not found in the orderbook");
    }

    auto& priceLevels = (ord.side == Side::Buy)
                       ? bids
                       : asks;

    priceLevels.erase(iter->second);
    orderMap.erase(ord.id);
  }

  void modify(const order& ord){
    auto iter = orderMap.find(ord.id);
    if(iter == orderMap.end()){
      throw std::runtime_error("Order not found in the orderbook");
    }

    auto& existingOrder = iter->second->second;
    if(existingOrder.id != ord.id || existingOrder.side != ord.side || existingOrder.symbol != ord.symbol){
      throw std::runtime_error("Invalid order received: not matching the existing order.");
    }

    if(existingOrder.price != ord.price){
      cancel(existingOrder);
      insert(ord);
    }

    existingOrder.quantity = ord.quantity;
  }

  void match(){
    while(!bids.empty() && !asks.empty()){
      auto bidIter = bids.rbegin();
      auto askIter = asks.begin();
      auto &[bestBidPrice, bidOrder] = *bidIter;
      auto &[bestAskPrice, askOrder] = *askIter;

      if(bestBidPrice < bestAskPrice){
        break;
      }

      auto matchingQuantity = std::min(bidOrder.quantity, askOrder.quantity);
      bidOrder.quantity -= matchingQuantity;
      askOrder.quantity -= matchingQuantity;

      if(bidOrder.quantity == 0)
        bids.erase(std::prev(bidIter.base()));

      if(askOrder.quantity == 0)
        asks.erase(askIter);
    } 
  }
};


void testOrderBook() {
  orderbook ordbook;

  // Insert Orders
  order o1 = {1, 10, 100.5, "AAPL", Side::Buy};
  order o2 = {2, 5, 101.0, "AAPL", Side::Sell};
  order o3 = {3, 8, 100.7, "AAPL", Side::Buy};
  order o4 = {4, 6, 100.8, "AAPL", Side::Sell};

  std::cout << "Inserting orders...\n";
  ordbook.insert(o1);
  ordbook.insert(o2);
  ordbook.insert(o3);
  ordbook.insert(o4);

  // Modify an order
  std::cout << "Modifying order 1...\n";
  order o1_mod = {1, 12, 100.5, "AAPL", Side::Buy};
  ordbook.modify(o1_mod);

  // Cancel an order
  std::cout << "Cancelling order 2...\n";
  ordbook.cancel(o2);

  // Matching orders
  std::cout << "Matching orders...\n";
  ordbook.match();
}

int main() {
  testOrderBook();
  return 0;
}


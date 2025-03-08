#include <list>
#include <unordered_map>

struct order{
  size_t id;
  double price;
  int quantity;
};

class lru {
 private:
  std::list<order> orders;
  std::unordered_map<size_t, std::list<order>::iterator> cache;
  size_t capacity = 5;

  /**
   * Moves an order to the front of the list.
   * - `std::list::splice` relinks the node to the front
   *   without invalidating its iterator.
   * - Since the iterator remains valid after splice,
   *   the mapping in the cache remains correct.
   */
  void moveToFront(std::list<order>::iterator orderIter) {
    orders.splice(orders.begin(), orders, orderIter);
  }

public:
  lru() = default;
  lru(size_t maxSize): capacity(maxSize)
  { }

  
  void put(const order& ord){
    auto iter = cache.find(ord.id);
    if(iter != cache.end()){
      auto &[orderID, orderIter] = *iter;
      moveToFront(orderIter);
    }else{
      if(cache.size() == capacity){
        auto lruOrder =  orders.back();
        orders.pop_back();
        cache.erase(lruOrder.id);
      }
      orders.push_front(ord);
      cache[ord.id]= orders.begin();
    }
  }

  const order* get(size_t orderID){
    auto iter = cache.find(orderID);
    if(iter == cache.end())
      return nullptr;
    else{
      auto&[orderID, orderIter] = *iter;
      moveToFront(orderIter);
      return &*orderIter;
    }
  }
};

int main() {
  return 0;  
}

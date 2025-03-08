#include <iostream>
#include <list>

struct Order {
    double price;
    int orderID;
    int quantity;
};

// Function to process a batch of orders
void processOrderBatch(Order* orderBatch[], size_t batchSize) {
    volatile double total = 0.0; // Prevent compiler optimizations
    for (size_t i = 0; i < batchSize; ++i) {
        total += orderBatch[i]->price * orderBatch[i]->quantity;
    }
}

// Function to process orders in batches with prefetching
void processOrdersInBatches(std::list<Order>& orders) {
    auto it = orders.begin();
    Order* orderBatch[4];

    while (it != orders.end()) {
        size_t batchSize = 0;

        // Prefetch the next 4 orders and collect them in a batch
        for (int i = 0; i < 4 && it != orders.end(); ++i, ++it) {
            __builtin_prefetch(&(*it), 0, 1);
            orderBatch[batchSize++] = &(*it);
        }

        // Process the batch of prefetched orders
        processOrderBatch(orderBatch, batchSize);
    }
}

int main() {
    std::list<Order> orders = {
        {100.5, 1, 2}, {200.0, 2, 1}, {150.75, 3, 4}, {300.25, 4, 3},
        {50.0, 5, 5}, {75.5, 6, 2}, {125.0, 7, 3}, {225.0, 8, 1}
    };
    
    processOrdersInBatches(orders);
    
    std::cout << "Processing complete." << std::endl;
    return 0;
}

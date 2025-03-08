#include <iostream>
#include <list>
#include <random>
#include <chrono>
#include <algorithm>

struct Order {
    double price;
    int orderID;
    int quantity;
};

void processOrder(const Order& order) {
    // Simulate processing
    volatile double total = order.price * order.quantity; // Prevent compiler optimizations
}

void processOrdersWithoutPrefetching(std::list<Order>& orders) {
    for (const auto& order : orders) {
        processOrder(order);
    }
}

int main() {
    const int numOrders = 1000000;
    std::list<Order> orders;
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> priceDist(10.0, 100.0);
    std::uniform_int_distribution<int> quantityDist(1, 10);

    for (int i = 0; i < numOrders; ++i) {
        orders.push_back({priceDist(rng), i, quantityDist(rng)});
    }

    auto start = std::chrono::high_resolution_clock::now();
    processOrdersWithoutPrefetching(orders);
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Processing time: " << std::chrono::duration<double>(end - start).count() << " seconds\n";
    return 0;
}

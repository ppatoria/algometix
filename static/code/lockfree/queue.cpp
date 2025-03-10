#include <atomic>
#include <iostream>

template <typename T, size_t Size>
class LockFreeQueue {
public:
    LockFreeQueue() : head(0), tail(0) {}

    bool enqueue(const T& item) {
        size_t current_tail = tail.load(std::memory_order_relaxed);
        size_t next_tail = increment(current_tail);

        if (next_tail == head.load(std::memory_order_acquire)) {
            return false; // Queue is full
        }

        buffer[current_tail] = item;
        tail.store(next_tail, std::memory_order_release);
        return true;
    }

    bool dequeue(T& item) {
        size_t current_head = head.load(std::memory_order_relaxed);

        if (current_head == tail.load(std::memory_order_acquire)) {
            return false; // Queue is empty
        }

        item = buffer[current_head];
        head.store(increment(current_head), std::memory_order_release);
        return true;
    }

private:
    size_t increment(size_t index) const {
        return (index + 1) % Size;
    }

    std::array<T, Size> buffer;
    std::atomic<size_t> head;
    std::atomic<size_t> tail;
};

int main() {
    LockFreeQueue<int, 10> queue;

    // Producer thread
    for (int i = 0; i < 10; ++i) {
        while (!queue.enqueue(i)) {
            // Queue is full, retry
        }
        std::cout << "Enqueued: " << i << std::endl;
    }

    // Consumer thread
    int value;
    for (int i = 0; i < 10; ++i) {
        while (!queue.dequeue(value)) {
            // Queue is empty, retry
        }
        std::cout << "Dequeued: " << value << std::endl;
    }

    return 0;
}

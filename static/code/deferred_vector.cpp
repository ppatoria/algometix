#include <vector>
#include <shared_mutex>
#include <utility>
#include "algorithm"
#include <mutex>

// Renamed class to deferred_vector to reflect optimized deletion handling
template <typename T>
class deferred_vector {
private:
    std::vector<std::pair<bool, T>> data; // Store active flag with data
    size_t deletionThreshold;
    size_t deletedCount = 0;
    mutable std::shared_mutex mutex;

public:
    explicit deferred_vector(size_t threshold = 10) 
        : deletionThreshold(threshold) {}

    void push_back(const T& value) {
        std::unique_lock lock(mutex);
        data.emplace_back(true, value);
    }
    
    void push_back(T&& value) {
        std::unique_lock lock(mutex);
        data.emplace_back(true, std::move(value));
    }
    
    void erase(size_t index) {
        std::unique_lock lock(mutex);
        if (index >= data.size() || !data[index].first) return;
        data[index].first = false;
        deletedCount++;
        if (deletedCount >= deletionThreshold) {
            compact();
        }
    }
    
    void compact() {
        std::unique_lock lock(mutex);
        data.erase(std::remove_if(data.begin(), data.end(), [](const std::pair<bool, T>& elem) {
            return !elem.first;
        }), data.end());
        deletedCount = 0;
    }
    
    void shrink_to_fit() { compact(); }
    
    size_t size() const {
        std::shared_lock lock(mutex);
        return data.size();
    }
    
    bool empty() const {
        std::shared_lock lock(mutex);
        return data.empty();
    }
    
    void clear() {
        std::unique_lock lock(mutex);
        data.clear(); deletedCount = 0;
    }
    
    T& operator[](size_t index) {
        std::shared_lock lock(mutex);
        return data[index].second;
    }
    
    const T& operator[](size_t index) const {
        std::shared_lock lock(mutex);
        return data[index].second;
    }
    
    T& at(size_t index) {
        std::shared_lock lock(mutex);
        if (index >= data.size()) throw std::out_of_range("Index out of range");
        return data[index].second;
    }
    
    const T& at(size_t index) const {
        std::shared_lock lock(mutex);
        if (index >= data.size()) throw std::out_of_range("Index out of range");
        return data[index].second;
    }
    
    typename std::vector<std::pair<bool, T>>::iterator begin() {
        std::shared_lock lock(mutex);
        return data.begin();
    }
    
    typename std::vector<std::pair<bool, T>>::iterator end() {
        std::shared_lock lock(mutex);
        return data.end();
    }
    
    typename std::vector<std::pair<bool, T>>::const_iterator begin() const {
        std::shared_lock lock(mutex);
        return data.begin();
    }
    
    typename std::vector<std::pair<bool, T>>::const_iterator end() const {
        std::shared_lock lock(mutex);
        return data.end();
    }
    
    void setDeletionThreshold(size_t threshold) {
        std::unique_lock lock(mutex);
        deletionThreshold = threshold;
    }
};

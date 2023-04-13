#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <queue>
#include <mutex>
#include <memory>

template <typename T, class base=std::queue<T>>
class Queue: base {
public:
    Queue(): base() {}
    Queue(const Queue &other) = delete;
    Queue(Queue &&other) = delete;

    void put(T value) {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        base::push(value);
    }

    void put(T &&value) {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        base::push(std::move(value));
    }

    T get() {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        T value = base::front();
        base::pop();
        return value;
    }

    bool empty() {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        return base::empty();
    }

private:
    std::mutex mutex_{};
};

#endif // QUEUE_HPP

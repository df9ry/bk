#ifndef QUEUE2_HPP
#define QUEUE2_HPP

#include <queue>
#include <mutex>
#include <memory>

template <typename T, class base=std::queue<T>>
class Queue2 {
public:
    Queue2() {}
    Queue2(const Queue2 &other) = delete;
    Queue2(Queue2 &&other) = delete;

    void put(T value, bool expedited = false) {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        expedited ? q_expedited.push(value)
                  : q_normal.push(value);
    }

    std::tuple<T, bool> get() {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        if (!q_expedited.empty()) {
            T value = std::move(q_expedited.front());
            q_expedited.pop();
            return { std::move(value), true };
        } else {
            T value = std::move(q_normal.front());
            q_normal.pop();
            return { std::move(value), false };
        }
    }

    bool empty() {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        return ( q_expedited.empty() && q_normal.empty() );
    }

private:
    std::queue<T> q_normal{};
    std::queue<T> q_expedited{};
    std::mutex    mutex_{};
};

#endif // QUEUE2_HPP

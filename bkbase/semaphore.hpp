#ifndef SEMAPHORE_HPP
#define SEMAPHORE_HPP

#include <mutex>
#include <condition_variable>

class semaphore
{
private:
    std::mutex mutex_;
    std::condition_variable condition_;
    unsigned long count_ = 0; // Initialized as locked.

public:
    void notify() {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        ++count_;
        condition_.notify_one();
    }

    void wait() {
        std::unique_lock<decltype(mutex_)> lock(mutex_);
        while(!count_) // Handle spurious wake-ups.
            condition_.wait(lock);
        --count_;
    }

    bool wait_for(int ms) {
        std::unique_lock<decltype(mutex_)> lock(mutex_);
        auto _t{std::chrono::steady_clock::now() +
                std::chrono::milliseconds(ms)};
        while(!count_) // Handle spurious wake-ups.
            if(condition_.wait_until(lock, _t) == std::cv_status::timeout)
                return false;
        --count_;
        return true;
    }

    bool try_wait() {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        if(count_) {
            --count_;
            return true;
        }
        return false;
    }
};

#endif // SEMAPHORE_HPP

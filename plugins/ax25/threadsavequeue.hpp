#ifndef THREADSAVEQUEUE_HPP
#define THREADSAVEQUEUE_HPP

#include <queue>
#include <condition_variable>
#include <mutex>

namespace ax25 {

    template<typename T>
    class ThreadSaveQueue: private std::queue<T>
    {
    public:
        ThreadSaveQueue() = default;
        ThreadSaveQueue(const ThreadSaveQueue &other) = delete;
        ThreadSaveQueue(ThreadSaveQueue &&other) = delete;

        bool empty() const
        {
            const std::lock_guard<std::mutex> lock(guard);
            return std::queue<T>::empty();
        }

        void clear()
        {
            const std::lock_guard<std::mutex> lock(guard);
            std::queue<T>::clear();
        }

        T get()
        {
            const std::lock_guard<std::mutex> lock(guard);
            return std::queue<T>::get();
        }

        void put(T x)
        {
            const std::lock_guard<std::mutex> lock(guard);
            std::queue<T>::put(x);
        }

    private:
        mutable std::mutex guard;

    };

} // end namespace ax25 //

#endif // THREADSAVEQUEUE_HPP

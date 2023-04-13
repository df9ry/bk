#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>
#include <functional>
#include <atomic>
#include <thread>
#include <condition_variable>

class Timer {

public:
    Timer();
    ~Timer();

    void Start(const std::chrono::milliseconds &interval,
               const std::function<void ()> &task,
               bool singleShot);
    void Start(const std::chrono::milliseconds &interval,
               const std::function<void ()> &task)
    {
        Start(interval, task, mSingleShot);
    }
    void Stop();
    void SetSingleShot(bool yes = true);
    bool GetSingleShot() const { return mSingleShot; }
    bool IsRunning() const;

private:
    std::chrono::milliseconds mInterval{};
    std::function<void ()> mTask{nullptr};
    std::atomic_bool mSingleShot{false};
    std::atomic_bool mRunning{false};
    std::condition_variable mRunCondition{};
    std::mutex mRunCondMutex{};
    std::thread mThread{};
    std::mutex mStopMutex{};
};

#endif // TIMER_HPP

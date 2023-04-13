
#include "timer.hpp"

Timer::Timer()
{
}

Timer::~Timer()
{
    Stop();
}

bool Timer::IsRunning() const
{
    return mRunning;
}

void Timer::Start(const std::chrono::milliseconds &interval,
                  const std::function<void ()> &task,
                  bool singleShot)
{
    Stop();

    mInterval = interval;
    mTask = task;
    mSingleShot = singleShot;
    mRunning = true;
    mThread = std::thread([this]
                  {
                      while (mRunning)
                      {
                          std::unique_lock<std::mutex> lock(mRunCondMutex);
                          auto waitResult = mRunCondition.wait_for(lock, mInterval, [this]{ return !mRunning; });
                          if (mRunning && !waitResult)
                              mTask();

                          if(mSingleShot)
                              mRunning = false;
                      }
                  });
}

void Timer::SetSingleShot(bool yes)
{
    mSingleShot = yes;
}

void Timer::Stop()
{
    std::unique_lock<std::mutex> lock(mStopMutex);
    mRunning = false;
    mRunCondition.notify_all();
    if(mThread.joinable())
        mThread.join();
}

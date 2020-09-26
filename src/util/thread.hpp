#ifndef SERVER_UTIL_THREAD_HPP_
#define SERVER_UTIL_THREAD_HPP_

#include <thread>

namespace util {

class ScopedThread
{
    std::thread thread_;

public:
    template<class... Args>
    explicit ScopedThread(Args&&... args)
      : thread_(std::forward<Args>(args)...)
    {}

    ScopedThread(ScopedThread&& other) { thread_ = std::move(other.thread_); }

    ~ScopedThread() { join(); }

    ScopedThread& operator=(ScopedThread&& other)
    {
        thread_ = std::move(other.thread_);
        return *this;
    }

    std::thread& operator*() { return thread_; }
    std::thread const& operator*() const { return thread_; }
    std::thread* operator->() { return &thread_; }
    std::thread const* operator->() const { return &thread_; }

    std::thread::id get_id() const { return thread_.get_id(); }

    void join()
    {
        if (thread_.joinable())
            thread_.join();
    }
};
}

#endif // SERVER_UTIL_THREAD_HPP_
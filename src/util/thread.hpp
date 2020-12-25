#include "pch.h"

#ifndef SERVER_UTIL_THREAD_HPP_
#define SERVER_UTIL_THREAD_HPP_

#include <thread>

namespace util {

class ScopedThread
{
    std::thread thread_;

public:
    ScopedThread() = default;
    ~ScopedThread() { join(); }

    ScopedThread(ScopedThread&& other) = delete;
    ScopedThread(const ScopedThread& other) = delete;

    template<class... Args>
    ScopedThread(Args&&... args)
      : thread_(std::forward<Args>(args)...)
    {}

    ScopedThread&
    operator=(std::thread&& other)
    {
        thread_ = std::move(other);
        return *this;
    }

    std::thread&
    operator*()
    {
        return thread_;
    }
    std::thread const&
    operator*() const
    {
        return thread_;
    }
    std::thread*
    operator->()
    {
        return &thread_;
    }
    std::thread const*
    operator->() const
    {
        return &thread_;
    }

    std::thread::id
    get_id() const
    {
        return thread_.get_id();
    }

    void
    join()
    {
        if (thread_.joinable())
            thread_.join();
    }
}; // class ScopedThread

} // namespace util

#endif // SERVER_UTIL_THREAD_HPP_
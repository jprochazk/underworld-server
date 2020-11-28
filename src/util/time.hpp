#ifndef SERVER_UTIL_TIME_HPP_
#define SERVER_UTIL_TIME_HPP_

#include <chrono>
#include <cstddef>
#include <iomanip>
#include <ratio>
#include <sstream>
#include <string>

namespace util {
namespace time {

namespace detail {

inline std::tm
localtime(const std::time_t& time)
{
    std::tm tm_snapshot;
#ifdef _WIN32
    localtime_s(&tm_snapshot, &time);
#else
    localtime_r(&time, &tm_snapshot);
#endif
    return tm_snapshot;
}
} // namespace detail

using Clock = std::chrono::system_clock;
using Duration = std::chrono::duration<double, std::milli>;
using Instant = std::chrono::time_point<Clock, Duration>;

struct Date : Instant
{
    std::string
    toString() const
    {
        auto tm_snapshot = detail::localtime(Clock::to_time_t(std::chrono::time_point<Clock, Clock::duration>(
          std::chrono::duration_cast<Clock::duration>(time_since_epoch()))));
        char buffer[32];
        std::strftime(buffer, sizeof(buffer), "%F %T %z", &tm_snapshot);
        return buffer;
    }
}; // struct Date

inline std::ostream&
operator<<(std::ostream& os, const Date& date)
{
    return os << date.toString();
}

inline Date
Now()
{
    return Date{ std::chrono::time_point<Clock, Duration>(
      std::chrono::duration_cast<Duration>(Clock::now().time_since_epoch())) };
}

} // namespace time
} // namespace util

#endif // SERVER_UTIL_TIME_HPP_
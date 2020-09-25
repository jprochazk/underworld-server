#ifndef SERVER_UTIL_LOG_HPP_
#define SERVER_UTIL_LOG_HPP_

#include <spdlog/spdlog.h>
#include <type_traits>

namespace util {
namespace log {

namespace detail {

template<typename E>
constexpr auto
to_integral(E e) -> typename std::underlying_type<E>::type
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}

}

enum class Level : int
{
    Trace = SPDLOG_LEVEL_TRACE,
    Debug = SPDLOG_LEVEL_DEBUG,
    Info = SPDLOG_LEVEL_INFO,
    Warn = SPDLOG_LEVEL_WARN,
    Err = SPDLOG_LEVEL_ERROR,
    Critical = SPDLOG_LEVEL_CRITICAL,
    Off = SPDLOG_LEVEL_OFF
};

inline void
SetLevel(Level level)
{
    spdlog::set_level(static_cast<spdlog::level::level_enum>(detail::to_integral(level)));
}

template<typename... Args>
inline void
Info(const std::string& origin, const std::string& format, Args&&... args)
{
    return spdlog::info("[{}] {}", origin, fmt::format(format, std::forward<Args>(args)...));
}

template<typename... Args>
inline void
Error(const std::string& origin, const std::string& format, Args&&... args)
{
    return spdlog::error("[{}] {}", origin, fmt::format(format, std::forward<Args>(args)...));
}

template<typename... Args>
inline void
Warn(const std::string& origin, const std::string& format, Args&&... args)
{
    return spdlog::warn("[{}] {}", origin, fmt::format(format, std::forward<Args>(args)...));
}

template<typename... Args>
inline void
Debug(const std::string& origin, const std::string& format, Args&&... args)
{
    return spdlog::debug("[{}] {}", origin, fmt::format(format, std::forward<Args>(args)...));
}

template<typename... Args>
inline void
Trace(const std::string& origin, const std::string& format, Args&&... args)
{
    return spdlog::trace("[{}] {}", origin, fmt::format(format, std::forward<Args>(args)...));
}

} // namespace log
} // namespace util

#endif // SERVER_UTIL_LOG_HPP_
#ifndef SERVER_UTIL_LOG_HPP_
#define SERVER_UTIL_LOG_HPP_

#include <spdlog/spdlog.h>
#include <type_traits>

namespace util {
namespace log {

namespace detail {

template<typename E>
constexpr auto
ToIntegral(E e) -> typename std::underlying_type<E>::type
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
    Error = SPDLOG_LEVEL_ERROR,
    Critical = SPDLOG_LEVEL_CRITICAL,
    Off = SPDLOG_LEVEL_OFF
};

// Default to "info"
inline std::string
ToString(Level level)
{
    static const std::unordered_map<util::log::Level, std::string> logLevels = {
        { util::log::Level::Trace, "trace" }, { util::log::Level::Debug, "debug" },
        { util::log::Level::Info, "info" },   { util::log::Level::Warn, "warn" },
        { util::log::Level::Error, "error" }, { util::log::Level::Critical, "fatal" }
    };
    if (auto it = logLevels.find(level); it != logLevels.end()) {
        return it->second;
    } else {
        return "info";
    }
}

// Default to Level::Info
inline util::log::Level
FromString(std::string level)
{
    const std::unordered_map<std::string, util::log::Level> logLevels = {
        { "trace", util::log::Level::Trace }, { "debug", util::log::Level::Debug },
        { "info", util::log::Level::Info },   { "warn", util::log::Level::Warn },
        { "error", util::log::Level::Error }, { "fatal", util::log::Level::Critical }
    };
    if (auto it = logLevels.find(level); it != logLevels.end()) {
        return it->second;
    } else {
        return util::log::Level::Info;
    }
}

inline void
SetLevel(Level level)
{
    spdlog::set_level(static_cast<spdlog::level::level_enum>(detail::ToIntegral(level)));
}

template<typename... Args>
inline void
Info(const std::string& origin, const std::string& format, Args&&... args)
{
    return spdlog::info("[{}] {}", origin, fmt::format(format, std::forward<Args>(args)...));
}

template<typename... Args>
inline void
Info(const std::string& format, Args&&... args)
{
    return spdlog::info(format, std::forward<Args>(args)...);
}

template<typename... Args>
inline void
Error(const std::string& origin, const std::string& format, Args&&... args)
{
    return spdlog::error("[{}] {}", origin, fmt::format(format, std::forward<Args>(args)...));
}

template<typename... Args>
inline void
Error(const std::string& format, Args&&... args)
{
    return spdlog::error(format, std::forward<Args>(args)...);
}

template<typename... Args>
inline void
Critical(const std::string& origin, const std::string& format, Args&&... args)
{
    return spdlog::critical("[{}] {}", origin, fmt::format(format, std::forward<Args>(args)...));
}

template<typename... Args>
inline void
Critical(const std::string& format, Args&&... args)
{
    return spdlog::critical(format, std::forward<Args>(args)...);
}

template<typename... Args>
inline void
Warn(const std::string& origin, const std::string& format, Args&&... args)
{
    return spdlog::warn("[{}] {}", origin, fmt::format(format, std::forward<Args>(args)...));
}

template<typename... Args>
inline void
Warn(const std::string& format, Args&&... args)
{
    return spdlog::warn(format, std::forward<Args>(args)...);
}

template<typename... Args>
inline void
Debug(const std::string& origin, const std::string& format, Args&&... args)
{
    return spdlog::debug("[{}] {}", origin, fmt::format(format, std::forward<Args>(args)...));
}

template<typename... Args>
inline void
Debug(const std::string& format, Args&&... args)
{
    return spdlog::debug(format, std::forward<Args>(args)...);
}

template<typename... Args>
inline void
Trace(const std::string& origin, const std::string& format, Args&&... args)
{
    return spdlog::trace("[{}] {}", origin, fmt::format(format, std::forward<Args>(args)...));
}

template<typename... Args>
inline void
Trace(const std::string& format, Args&&... args)
{
    return spdlog::trace(format, std::forward<Args>(args)...);
}

} // namespace log
} // namespace util

#endif // SERVER_UTIL_LOG_HPP_
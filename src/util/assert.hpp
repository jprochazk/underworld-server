#ifndef SERVER_UTIL_ASSERT_HPP_
#define SERVER_UTIL_ASSERT_HPP_

#include <stdexcept>

namespace util {

// I know it's not actually namespaced but it makes it feel less dirty

#ifdef NDEBUG
#    define ASSERT(expr) (static_cast<void>(expr));
#else
#    define ASSERT(expr)                                                                                               \
        if (!static_cast<bool>(expr))                                                                                  \
            throw std::runtime_error{ "Assertion failed: "##expr };
#endif

} // namespace util

#endif // SERVER_UTIL_ASSERT_HPP_
/**
 *
 * This file defines:
 *  - Native byte order
 *  - Byte swap intrinsics (with fallbacks)
 *  - Endian conversion functions
 *
 * Much of this code is originally from Boost.Endian
 * Original work Copyright (c) 2019 Peter Dimov distributed under the http://www.boost.org/LICENSE_1_0.txt
 * Modified work Copyright (c) 2020 Jan Procházka re-distributed under the MIT license (see LICENSE in the repository
 * root)
 */

#ifndef SERVER_ENDIAN_HPP_
#define SERVER_ENDIAN_HPP_

#define LITTLE_ENDIAN_ORDER 0
#define BIG_ENDIAN_ORDER 1
#define NETWORK_ORDER BIG_ENDIAN_ORDER

#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#    define NATIVE_ORDER LITTLE_ENDIAN_ORDER
#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#    define NATIVE_ORDER BIG_ENDIAN_ORDER
#elif defined(__BYTE_ORDER__) && defined(__ORDER_PDP_ENDIAN__) && __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__
#    error Platforms with PDP endianness are not supported.
#elif defined(__LITTLE_ENDIAN__)
#    define NATIVE_ORDER LITTLE_ENDIAN_ORDER
#elif defined(__BIG_ENDIAN__)
#    define NATIVE_ORDER BIG_ENDIAN
#elif defined(_MSC_VER) || defined(__i386__) || defined(__x86_64__)
#    define NATIVE_ORDER LITTLE_ENDIAN_ORDER
#else
#    error Could not determine platform endianness.
#endif

#ifndef ENDIAN_NO_INTRINSICS

#    ifndef __has_builtin
#        define __has_builtin(x) 0 // Compatibility with non-clang compilers
#    endif

#    if defined(_MSC_VER) && (!defined(__clang__) || defined(__c2__))
#        define ENDIAN_INTRINSIC_MSG "cstdlib _byteswap_ushort, etc."
#        include <cstdlib>
#        define ENDIAN_INTRINSIC_BYTE_SWAP_2(x) _byteswap_ushort(x)
#        define ENDIAN_INTRINSIC_BYTE_SWAP_4(x) _byteswap_ulong(x)
#        define ENDIAN_INTRINSIC_BYTE_SWAP_8(x) _byteswap_uint64(x)

//  GCC and Clang recent versions provide intrinsic byte swaps via builtins
#    elif (defined(__clang__) && __has_builtin(__builtin_bswap32) && __has_builtin(__builtin_bswap64)) ||              \
      (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)))
#        define ENDIAN_INTRINSIC_MSG "__builtin_bswap16, etc."
// prior to 4.8, gcc did not provide __builtin_bswap16 on some platforms so we emulate it
// see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=52624
// Clang has a similar problem, but their feature test macros make it easier to detect
#        if (defined(__clang__) && __has_builtin(__builtin_bswap16)) ||                                                \
          (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)))
#            define ENDIAN_INTRINSIC_BYTE_SWAP_2(x) __builtin_bswap16(x)
#        else
#            define ENDIAN_INTRINSIC_BYTE_SWAP_2(x) __builtin_bswap32((x) << 16)
#        endif
#        define ENDIAN_INTRINSIC_BYTE_SWAP_4(x) __builtin_bswap32(x)
#        define ENDIAN_INTRINSIC_BYTE_SWAP_8(x) __builtin_bswap64(x)

#        define ENDIAN_CONSTEXPR_INTRINSICS

//  Linux systems provide the byteswap.h header, with
#    elif defined(__linux__)
//  don't check for obsolete forms defined(linux) and defined(__linux) on the theory that
//  compilers that predefine only these are so old that byteswap.h probably isn't present.
#        define ENDIAN_INTRINSIC_MSG "byteswap.h bswap_16, etc."
#        include <byteswap.h>
#        define ENDIAN_INTRINSIC_BYTE_SWAP_2(x) bswap_16(x)
#        define ENDIAN_INTRINSIC_BYTE_SWAP_4(x) bswap_32(x)
#        define ENDIAN_INTRINSIC_BYTE_SWAP_8(x) bswap_64(x)

#    else
#        define ENDIAN_NO_INTRINSICS
#        define ENDIAN_INTRINSIC_MSG "no byte swap intrinsics"
#    endif

#elif !defined(ENDIAN_INTRINSIC_MSG)
#    define ENDIAN_INTRINSIC_MSG "no byte swap intrinsics"
#endif // ENDIAN_NO_INTRINSICS

#include "util/log.hpp"
#include <cstdint>
#include <cstring>
#include <type_traits>

namespace endian {

namespace detail {

template<std::size_t N>
struct integral_by_size
{};

template<>
struct integral_by_size<1>
{
    typedef uint8_t type;
};

template<>
struct integral_by_size<2>
{
    typedef uint16_t type;
};

template<>
struct integral_by_size<4>
{
    typedef uint32_t type;
};

template<>
struct integral_by_size<8>
{
    typedef uint64_t type;
};

template<class T>
struct negation : std::integral_constant<bool, !T::value>
{};

template<class T>
struct is_scoped_enum
  : std::conditional<std::is_enum<T>::value, negation<std::is_convertible<T, int>>, std::false_type>::type
{};

inline uint8_t constexpr reverse_impl(uint8_t x) noexcept
{
    return x;
}

inline uint16_t constexpr reverse_impl(uint16_t x) noexcept
{
#ifdef ENDIAN_NO_INTRINSICS
    return (x << 8) | (x >> 8);
#else
    return ENDIAN_INTRINSIC_BYTE_SWAP_2(x);
#endif
}

inline uint32_t constexpr reverse_impl(uint32_t x) noexcept
{
#ifdef ENDIAN_NO_INTRINSICS
    uint32_t step16 = x << 16 | x >> 16;
    return ((step16 << 8) & 0xff00ff00) | ((step16 >> 8) & 0x00ff00ff);
#else
    return ENDIAN_INTRINSIC_BYTE_SWAP_4(x);
#endif
}

inline uint64_t constexpr reverse_impl(uint64_t x) noexcept
{
#ifdef ENDIAN_NO_INTRINSICS
    uint64_t step32 = x << 32 | x >> 32;
    uint64_t step16 = (step32 & 0x0000FFFF0000FFFF_ULL) << 16 | (step32 & 0xFFFF0000FFFF0000_ULL) >> 16;
    return (step16 & 0x00FF00FF00FF00FF_ULL) << 8 | (step16 & 0xFF00FF00FF00FF00_ULL) >> 8;
#else
    return ENDIAN_INTRINSIC_BYTE_SWAP_8(x);
#endif
}

template<class N>
struct is_reversible
  : std::integral_constant<bool,
                           (std::is_integral<N>::value && !std::is_same<N, bool>::value) || is_scoped_enum<N>::value>
{};

template<class N>
struct is_reversible_inplace
  : std::integral_constant<bool,
                           std::is_integral<N>::value || std::is_enum<N>::value || std::is_same<N, float>::value ||
                             std::is_same<N, double>::value>
{};

} // namespace detail

// Requires:
//   T is non-bool integral or scoped enumeration type

template<class T>
inline constexpr typename std::enable_if<!std::is_class<T>::value, T>::type
reverse(T x) noexcept
{
    static_assert(detail::is_reversible<T>::value);

    typedef typename detail::integral_by_size<sizeof(T)>::type uintN_t;

    return static_cast<T>(detail::reverse_impl(static_cast<uintN_t>(x)));
}

// Requires:
//   T is integral, enumeration, float or double

template<class T>
inline typename std::enable_if<!std::is_class<T>::value>::type
reverse_inplace(T& x) noexcept
{
    static_assert(detail::is_reversible_inplace<T>::value);

    typename detail::integral_by_size<sizeof(T)>::type x2;

    std::memcpy(&x2, &x, sizeof(T));

    x2 = detail::reverse_impl(x2);

    std::memcpy(&x, &x2, sizeof(T));
}

// Default implementation for user-defined types

template<class T>
inline typename std::enable_if<std::is_class<T>::value>::type
reverse_inplace(T& x) noexcept
{
    x = reverse(x);
}

// reverse_inplace for arrays

template<class T, std::size_t N>
inline void reverse_inplace(T (&x)[N]) noexcept
{
    for (std::size_t i = 0; i < N; ++i) {
        reverse_inplace(x[i]);
    }
}

// This is dead code, but it may be useful in the future.
/*
template<class T>
inline void
network_order(T& x) noexcept
{
#if NATIVE_ORDER != NETWORK_ORDER
    reverse_inplace(x);
#else
    (void)x;
#endif
}

template<class T, std::size_t N>
inline void network_order_array(T (&x)[N]) noexcept
{
    for (std::size_t i = 0; i < N; ++i) {
        network_order(x[i]);
    }
}

template<class T>
inline void
network_order_range(T* data, std::size_t size) noexcept
{
    for (std::size_t i = 0; i < size; ++i) {
        network_order(data[i]);
    }
}

template<class T>
inline void
native_order(T& x) noexcept
{
#if NATIVE_ORDER != NETWORK_ORDER
    if constexpr (std::is_enum_v<T>) {
        if constexpr (std::is_same_v<std::underlying_type_t<T>, uint16_t>) {
            util::log::Info("TEST", "It's uint16_t...");
        }
    }
    reverse_inplace(x);
#else
    (void)x;
#endif
}

template<class T, std::size_t N>
inline void native_order_array(T (&x)[N]) noexcept
{
    for (std::size_t i = 0; i < N; ++i) {
        native_order(x[i]);
    }
}

template<class T>
inline void
native_order_range(T* data, std::size_t size) noexcept
{
    for (std::size_t i = 0; i < size; ++i) {
        native_order(data[i]);
    }
} */

} // namespace endian

#endif // SERVER_ENDIAN_HPP_
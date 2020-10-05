#ifndef SERVER_NET_PACKET_HPP_
#define SERVER_NET_PACKET_HPP_

//#include "net/endian.hpp"
//#include <boost/fusion/algorithm.hpp>
//#include <boost/fusion/include/algorithm.hpp>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace net {

template<typename T>
bool
Deserialize(uint8_t* data, size_t size, T* out)
{
    if (size < sizeof(T))
        return false;

    // ensure data is initialized
    *out = T{};

    // endianness conversion is complex to do in a maintainable way, without byte buffer abstractions (clunky)
    // or reflection (c++ doesn't have it, but JS does!)

    // so we'll be using little-endian here, and on the client-side, we'll ensure data that comes out
    // is little-endian, and what comes in is native-endian (big or little, who knows)

    // considering the fact that the game will most likely only be played on windows,
    // which is ALWAYS little-endian, even that could be omitted without much harm.
    // Linux can run on big-endian, but it is uncommon, and there are no plans for MacOS support, due
    // to their lacking support of WebGL2 (among other web standards)

    // ensure correct endianness
    // if constexpr (boost::fusion::traits::is_sequence<T>::value) {
    //    util::log::Info("Deserialize", "Type {} as sequence", typeid(T).name());
    //    boost::fusion::for_each(*out, endian::native_swap{});
    //} else {
    //    util::log::Info("Deserialize", "Type {} as non-sequence", typeid(T).name());
    //    endian::native_order(*out);
    //}

    // copy data into packet
    std::memcpy(out, data, sizeof(T));
    return true;
}

} // namespace net

#endif // SERVER_NET_PACKET_HPP_

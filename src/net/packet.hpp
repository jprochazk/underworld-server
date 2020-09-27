#ifndef SERVER_NET_PACKET_HPP_
#define SERVER_NET_PACKET_HPP_

#include "endian.hpp"
#include <cstddef>
#include <cstdint>

namespace net {

template<typename T>
bool
Deserialize(size_t size, uint8_t* data, T* out)
{
    if (size < sizeof(T))
        return false;

    // ensure correct endianness
    endian::native_order_range(size, data);
    // for (size_t i = 0; i < sizeof(T); ++i) {
    //    endian::native_order(data[i]);
    //}

    // TODO: find out if this copy can be avoided
    // copy data into packet
    std::memcpy(out, data, sizeof(T));
    return true;
}

} // namespace net

#endif // SERVER_NET_PACKET_HPP_

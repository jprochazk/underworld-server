#ifndef SERVER_NET_PACKET_HPP_
#define SERVER_NET_PACKET_HPP_

#include "boost/pfr/precise/core.hpp"
#include "net/endian.hpp"
#include "boost/pfr.hpp"
#include "util/log.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <vector>

namespace net {

class Packet
{
public:
    Packet()
      : cursor_{ 0 }
      , buffer_{}
    {}
    Packet(std::vector<uint8_t>&& buffer)
      : cursor_{ 0 }
      , buffer_{ std::move(buffer) }
    {}
    Packet(uint8_t* data, size_t size)
      : cursor_{ 0 }
      , buffer_{ data, data + size }
    {}

    template<typename T>
    friend Packet&
    operator>>(Packet& packet, T& data)
    {
        static_assert(std::is_fundamental_v<T> || std::is_pod_v<T>, "Data must be fundamental or POD.");

        if constexpr (std::is_fundamental_v<T>) {
            std::memcpy(&data, packet.buffer_.data() + packet.cursor_, sizeof(T));
            endian::reverse_inplace(data);
            packet.cursor_ += sizeof(T);
        } else if constexpr (std::is_pod_v<T>) {
            boost::pfr::for_each_field(data, [&](auto& value) {
                std::memcpy(&value, packet.buffer_.data() + packet.cursor_, sizeof(value));
                endian::reverse_inplace(value);
                packet.cursor_ += sizeof(value);
            });
        }

        return packet;
    }

    template<typename T>
    friend Packet&
    operator<<(Packet& packet, const T& data)
    {
        static_assert(std::is_fundamental_v<T> || std::is_pod_v<T>, "Data must be fundamental or POD.");

        if (packet.buffer_.size() - packet.cursor_ < sizeof(T)) {
            packet.buffer_.resize(packet.cursor_ + sizeof(T));
        }

        if constexpr (std::is_fundamental_v<T>) {
            std::memcpy(packet.buffer_.data() + packet.cursor_, &data, sizeof(T));
            endian::reverse_inplace(*(reinterpret_cast<T*>(packet.buffer_.data() + packet.cursor_)));
            packet.cursor_ += sizeof(T);
        } else if constexpr (std::is_pod_v<T>) {
            boost::pfr::for_each_field(data, [&](const auto& value) {
                std::memcpy(packet.buffer_.data() + packet.cursor_, &value, sizeof(value));
                endian::reverse_inplace(
                  *(reinterpret_cast<std::remove_const_t<std::remove_reference_t<decltype(value)>>*>(
                    packet.buffer_.data() + packet.cursor_)));
                packet.cursor_ += sizeof(value);
            });
        }

        return packet;
    }

    size_t
    cursor() const
    {
        return cursor_;
    }

    // Analogous to `std::vector<uint8_t>::size() const`
    size_t
    size() const
    {
        return buffer_.size();
    }

    // Analogous to `std::vector<uint8_t>::resize(n)`
    void
    resize(size_t n)
    {
        return buffer_.resize(n);
    }

    // Analogous to `std::vector<uint8_t>::empty() const`
    bool
    empty() const
    {
        return buffer_.empty();
    }

    // Analogous to `std::vector<uint8_t>::data()`
    uint8_t*
    data()
    {
        return buffer_.data();
    }

    // Analogous to `std::vector<uint8_t>::data() const`
    const uint8_t*
    data() const
    {
        return buffer_.data();
    }

    // Analogous to `std::vector<uint8_t>::begin()`
    std::vector<uint8_t>::iterator
    begin()
    {
        return buffer_.begin();
    }

    // Analogous to `std::vector<uint8_t>::end()`
    std::vector<uint8_t>::iterator
    end()
    {
        return buffer_.end();
    }

private:
    size_t cursor_;
    std::vector<uint8_t> buffer_;
};

} // namespace net

#endif // SERVER_NET_PACKET_HPP_

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
    // Move-construct from a std::vector
    Packet(std::vector<uint8_t>&& buffer)
      : cursor_{ 0 }
      , buffer_{ std::move(buffer) }
    {}
    // Copy-construct from a range
    Packet(uint8_t* data, size_t size)
      : cursor_{ 0 }
      , buffer_{ data, data + size }
    {}

    // Allows for deserializing fundamental types (int, float, etc) 
    // or POD types (standard layout and trivially constructible, such as C-structs)
    template<typename T>
    Packet&
    read(T& data)
    {
        static_assert(std::is_fundamental_v<T> || std::is_pod_v<T>, "Data must be fundamental or POD.");

        if constexpr (std::is_fundamental_v<T>) {
            // if we're deserializing a fundamental type
            // copy the data from the internal buffer
            std::memcpy(&data, buffer_.data() + cursor_, sizeof(T));
            // reverse its endianness if on a little-endian platform
            endian::reverse_inplace(data);
            // move the cursor forward
            cursor_ += sizeof(T);
        } else if constexpr (std::is_pod_v<T>) {
            // if we're deserializing a POD type
            // iterate over each field in the POD - this can be a tuple, a plain array, or a struct
            // deserializing each field individually
            boost::pfr::for_each_field(data, [&](auto& value) {
                // TODO: de-duplicate this code
                // doing the same as above, with sizeof(value) instead of sizeof(T)
                std::memcpy(&value, buffer_.data() + cursor_, sizeof(value));
                endian::reverse_inplace(value);
                cursor_ += sizeof(value);
            });
        }

        return *this;
    }

    // Allows for serializing fundamental types (int, float, etc) 
    // or POD types (standard layout and trivially constructible, such as C-structs)
    template<typename T>
    Packet&
    write(const T& data)
    {
        static_assert(std::is_fundamental_v<T> || std::is_pod_v<T>, "Data must be fundamental or POD.");

        // If we don't have enough space, allocate more
        if (buffer_.size() - cursor_ < sizeof(T)) {
            buffer_.resize(cursor_ + sizeof(T));
        }

        if constexpr (std::is_fundamental_v<T>) {
            // If we're serializing a fundamental type
            // copy the value into the internal buffer
            std::memcpy(buffer_.data() + cursor_, &data, sizeof(T));
            // convert the endianness of what we just copied into the buffer
            // here we're aliasing the buffer data pointer
            endian::reverse_inplace(*(reinterpret_cast<T*>(buffer_.data() + cursor_)));
            // move the cursor forward
            cursor_ += sizeof(T);
        } else if constexpr (std::is_pod_v<T>) {
            boost::pfr::for_each_field(data, [&](const auto& value) {
                // TODO: de-duplicate this code
                // doing the same thing as above, just with sizeof(value) instead of sizeof(T)
                std::memcpy(buffer_.data() + cursor_, &value, sizeof(value));
                endian::reverse_inplace(
                  *(reinterpret_cast<std::remove_const_t<std::remove_reference_t<decltype(value)>>*>(
                    buffer_.data() + cursor_)));
                cursor_ += sizeof(value);
            });
        }

        return *this;
    }

    // Returns the internal cursor position
    size_t
    cursor() const
    {
        return cursor_;
    }

    // Set the internal cursor position (at your own peril)
    size_t
    cursor(size_t pos)
    {
        return cursor_ = pos;
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

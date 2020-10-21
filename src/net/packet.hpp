#ifndef SERVER_NET_PACKET_HPP_
#define SERVER_NET_PACKET_HPP_

#include "net/endian.hpp"
#include "util/log.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <vector>

#ifndef NDEBUG
#    include <stdexcept>
#endif

namespace net {

class Packet
{
public:
    Packet()
      : cursor_{ 0 }
      , buffer_{}
    {}
    Packet(size_t size)
      : cursor_{ 0 }
      , buffer_(size)
    {}

    Packet(std::initializer_list<uint8_t> bytes)
      : cursor_{ 0 }
      , buffer_(bytes)
    {}
    Packet&
    operator=(std::initializer_list<uint8_t> bytes)
    {
        buffer_ = bytes;
        return *this;
    }

    // Move-construct from a std::vector
    Packet(std::vector<uint8_t>&& buffer)
      : cursor_{ 0 }
      , buffer_{ std::move(buffer) }
    {}
    Packet&
    operator=(std::vector<uint8_t>&& buffer)
    {
        if (&buffer_ != &buffer) {
            buffer_ = std::move(buffer);
        }
        return *this;
    }

    // Copy-construct from a range
    Packet(uint8_t* data, size_t size)
      : cursor_{ 0 }
      , buffer_{ data, data + size }
    {}

    template<typename T>
    void
    read(T& data, size_t size)
    {
        std::memcpy(&data, buffer_.data() + cursor_, size);
        cursor_ += size;
    }

    template<typename T>
    void
    read(T& data)
    {
        read(data, sizeof(T));
    }

    template<typename T>
    void
    write(const T& data, size_t size)
    {
        buffer_.resize(cursor_ + size);
        std::memcpy(buffer_.data() + cursor_, &data, size);
        cursor_ += size;
    }

    template<typename T>
    void
    write(const T& data)
    {
        write(data, sizeof(T));
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

    size_t
    remaining() const
    {
        return size() - cursor();
    }

    inline bool
    operator==(const net::Packet& other) const
    {
        return (buffer_ == other.buffer_);
    }

    inline uint8_t& operator[](size_t pos) { return buffer_[pos]; }
    inline uint8_t operator[](size_t pos) const { return buffer_[pos]; }

    inline operator std::vector<uint8_t>() { return buffer_; }

private:
    size_t cursor_;
    std::vector<uint8_t> buffer_;
}; // class Packet

template<typename T>
inline bool
Deserialize(Packet& packet, T& data)
{
    static_assert(std::is_arithmetic_v<T> || (std::is_standard_layout_v<T> && std::is_trivial_v<T>),
                  "Data is not trivial and has no better deserialize overload");
    if (packet.remaining() < sizeof(T))
        return false;
    packet.read(data);
    return true;
}

template<typename T>
inline bool
Deserialize(Packet& packet, std::basic_string<T>& str)
{
    uint16_t count;
    if (!net::Deserialize(packet, count))
        return false;

    // NOTE: this is entirely arbitrary
    if (count > 1024)
        return false;

    str.resize(count);
    std::memcpy(str.data(), packet.data() + packet.cursor(), count);
    packet.cursor(packet.cursor() + (str.size() * sizeof(T)));
    return true;
}

template<typename T>
inline bool
Deserialize(Packet& packet, std::vector<T>& vec)
{
    uint16_t count;
    if (!net::Deserialize(packet, count))
        return false;

    // NOTE: this is entirely arbitrary
    if (count > 1024)
        return false;

    vec.resize(count);
    for (auto& el : vec) {
        Deserialize(packet, el);
    }
    return true;
}

template<typename T>
inline void
Serialize(Packet& packet, const T& data)
{
    static_assert(std::is_arithmetic_v<T> || (std::is_standard_layout_v<T> && std::is_trivial_v<T>),
                  "Data is not trivial and has no better serialize overload");
    packet.write(data);
}

// extra definitions for basic_string and vector
template<typename T>
inline void
Serialize(Packet& packet, const std::basic_string<T>& str)
{
    packet.write(static_cast<uint16_t>(str.size()));

    packet.resize(packet.cursor() + str.size());
    std::memcpy(packet.data() + packet.cursor(), str.data(), str.size());
    packet.cursor(packet.cursor() + (str.size() * sizeof(T)));
}

template<typename T>
inline void
Serialize(Packet& packet, const std::vector<T>& vec)
{
    packet.write(static_cast<uint16_t>(vec.size()));

    for (auto& el : vec) {
        Serialize(packet, el);
    }
}

} // namespace net

#endif // SERVER_NET_PACKET_HPP_

#include "pch.h"

#ifndef SERVER_UTIL_JSON_HPP_
#define SERVER_UTIL_JSON_HPP_

#include "log.hpp"

// TODO: document this

namespace nlohmann {

template<typename T>
struct adl_serializer<std::optional<T>>
{
    static void
    to_json(json& j, const std::optional<T>& opt)
    {
        if (!opt.has_value()) {
            j = nullptr;
        } else {
            j = *opt;
        }
    }

    static void
    from_json(const json& j, std::optional<T>& opt)
    {
        if (j.is_null()) {
            opt = {};
        } else {
            opt = j.get<T>();
        }
    }
}; // struct adl_serializer<std::optional<T>>

template<typename T, typename E = void>
inline bool
is_json_t(const json& json)
{
    if constexpr (std::is_same_v<bool, T>) {
        return json.is_boolean();
    } else if constexpr (std::is_same_v<float, T>) {
        return json.is_number_float();
    } else if constexpr (std::is_same_v<int32_t, T>) {
        return json.is_number_integer();
    } else if constexpr (std::is_same_v<int16_t, T>) {
        return json.is_number_integer();
    } else if constexpr (std::is_same_v<int8_t, T>) {
        return json.is_number_integer();
    } else if constexpr (std::is_same_v<uint32_t, T>) {
        return json.is_number_unsigned();
    } else if constexpr (std::is_same_v<uint16_t, T>) {
        return json.is_number_unsigned();
    } else if constexpr (std::is_same_v<uint8_t, T>) {
        return json.is_number_unsigned();
    } else if constexpr (std::is_class_v<T> && std::is_same_v<std::string, T>) {
        return json.is_string();
    } else if constexpr (std::is_class_v<T> && std::is_same_v<std::vector<uint8_t>, std::vector<E>>) {
        return json.is_binary();
    } else if constexpr (std::is_class_v<T> && std::is_same_v<std::vector<E>, T>) {
        return json.is_array();
    }
}

} // namespace nlohmann

// Set `value` to `property` if `property` exists
// Does nothing if `property` doesn't exist
// Used with std::optional
template<typename T>
inline void
try_get_to(const json& json, std::string_view name, T& value)
{
    auto v = json.find(name);
    if (v != json.end()) {
        if constexpr (std::is_class_v<T>) {
            if (!nlohmann::is_json_t<T, typename T::value_type>(*v))
                return;
        } else {
            if (!nlohmann::is_json_t<T>(*v))
                return;
        }
        v->get_to(value);
    }
}

// Get a property or return default `value`
template<typename T>
inline T
try_get_default(const json& json, std::string_view name, const T& value)
{
    auto v = json.find(name);
    if (v != json.end()) {
        if constexpr (std::is_class_v<T>) {
            if (!nlohmann::is_json_t<T, typename T::value_type>(*v))
                return value;
        } else {
            if (!nlohmann::is_json_t<T>(*v))
                return value;
        }
        return v->get<T>();
    }
    return value;
}

namespace util {

json inline LoadJson(const std::string& path)
{
    log::Debug("LoadJson", "Loading \"{}\"", path);
    return json::parse(std::ifstream(path), nullptr, false);
}

} // namespace util

#endif // SERVER_UTIL_JSON_HPP_
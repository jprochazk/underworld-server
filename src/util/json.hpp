#ifndef SERVER_UTIL_JSON_HPP_
#define SERVER_UTIL_JSON_HPP_

#include "log.hpp"
#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>
using json = nlohmann::json;

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

template<typename T, typename U = void>
inline bool
is_json_t(const json& json, T)
{
    return json.is_null() || json.is_discarded();
}
template<typename T>
inline bool
is_json_t(const json& json, std::vector<T>)
{
    return json.is_array();
}
template<>
inline bool
is_json_t(const json& json, std::vector<uint8_t>)
{
    return json.is_binary();
}
template<>
inline bool
is_json_t(const json& json, std::string)
{
    return json.is_string();
}
template<>
inline bool
is_json_t(const json& json, uint8_t)
{
    return json.is_number_unsigned();
}
template<>
inline bool
is_json_t(const json& json, uint16_t)
{
    return json.is_number_unsigned();
}
template<>
inline bool
is_json_t(const json& json, uint32_t)
{
    return json.is_number_unsigned();
}
template<>
inline bool
is_json_t(const json& json, int8_t)
{
    return json.is_number_integer();
}
template<>
inline bool
is_json_t(const json& json, int16_t)
{
    return json.is_number_integer();
}
template<>
inline bool
is_json_t(const json& json, int32_t)
{
    return json.is_number_integer();
}
template<>
inline bool
is_json_t(const json& json, float)
{
    return json.is_number_float();
}
template<>
inline bool
is_json_t(const json& json, bool)
{
    return json.is_boolean();
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
        if (!nlohmann::is_json_t<T>(*v, {}))
            return;
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
        if (!nlohmann::is_json_t<T>(*v, {}))
            return value;
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
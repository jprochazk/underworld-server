#ifndef SERVER_PCH_H_
#define SERVER_PCH_H_

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <istream>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <filesystem>
namespace fs = std::filesystem;

#include <boost/asio.hpp>
#include <boost/beast.hpp>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;
namespace beast = boost::beast;
namespace http = beast::http;

#include <spdlog/spdlog.h>

#include <entt/entt.hpp>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "sol/forward.hpp"
#ifndef NDEBUG
#define SOL_EXCEPTIONS_SAFE_PROPAGATION 1
#define SOL_ALL_SAFETIES_ON 1
#endif
#define SOL_USE_BOOST 1
#include <sol/sol.hpp>

#endif // SERVER_PCH_H_
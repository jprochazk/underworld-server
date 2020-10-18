#include "game/script.hpp"
#include "util/log.hpp"
#include "util/time.hpp"
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <filesystem>
namespace fs = std::filesystem;

namespace game {

namespace script {

// Globally, I need to store two things:
// 1. Script bytecode
// 2. API bindings

// To test the system, try to return the current time from a script,
// where the script calls a C++ function to retrieve the time as a string

struct
{
    sol::state state;
    struct
    {
        std::shared_mutex mutex;
        std::unordered_map<std::string, sol::bytecode> storage;
    } scripts;
} global;

void
Load(const std::string& path)
{
    fs::path file{ path };

    if (!fs::exists(file)) {
        throw std::runtime_error{ fmt::format("{} does not exist", path) };
    }

    if (fs::is_directory(file)) {
        for (auto it = fs::directory_iterator{ file }; it != fs::directory_iterator{}; ++it) {
            if (it->is_directory()) {
                Load(it->path().string());
            } else if (it->is_regular_file() && it->path().has_extension() &&
                       it->path().extension() == fs::path{ ".lua" }) {
                std::unique_lock lock{ global.scripts.mutex };

                auto path = it->path().string();
                auto result = global.state.load_file(path);
                if (!result.valid()) {
                    throw std::runtime_error{ fmt::format("File \"{}\" is not a valid script", path) };
                }

                global.scripts.storage.emplace(path, (sol::function{ result }).dump());
            }
        }
    } else if (fs::is_regular_file(file)) {
        std::unique_lock lock{ global.scripts.mutex };

        auto path = file.string();
        auto result = global.state.load_file(path);
        if (!result.valid()) {
            throw std::runtime_error{ fmt::format("File \"{}\" is not a valid script", path) };
        }

        global.scripts.storage.emplace(path, (sol::function{ result }).dump());
    }
}

sol::function
Get(const std::string& path, sol::state& state)
{
    std::shared_lock lock{ global.scripts.mutex };

    if (auto it = global.scripts.storage.find(path); it != global.scripts.storage.end()) {
        return state.load(it->second.as_string_view());
    } else {
        throw std::runtime_error{ fmt::format("Script \"{}\" not found", path) };
    }
}

void
Initialize(sol::state& state)
{
    state.open_libraries();

    sol::table game = state.create_table("game");

    sol::table util = state.create_table("util");
    sol::table time = util.create("time");
    time.new_usertype<util::time::Date>("Date");
    time.new_usertype<util::time::Duration>("Duration");
    time["now"] = [&]() -> util::time::Date { return util::time::Now(); };

    sol::table log = util.create("log");
    log["trace"] = [&](std::string message) -> void { util::log::Trace("LUA", "{}", message); };
    log["debug"] = [&](std::string message) -> void { util::log::Debug("LUA", "{}", message); };
    log["info"] = [&](std::string message) -> void { util::log::Info("LUA", "{}", message); };
    log["warn"] = [&](std::string message) -> void { util::log::Warn("LUA", "{}", message); };
    log["error"] = [&](std::string message) -> void { util::log::Error("LUA", "{}", message); };
    log["critical"] = [&](std::string message) -> void { util::log::Critical("LUA", "{}", message); };
}

} // namespace script

} // namespace game
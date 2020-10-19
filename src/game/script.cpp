#include "game/script.hpp"
#include "sol/forward.hpp"
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

void
Initialize(sol::state& state)
{
    state.open_libraries();

    sol::table game = state.create_table("game");

    sol::table util = state.create_table("util");

    // util.time
    sol::table time = util.create("time");
    time.new_usertype<util::time::Date>("Date");
    time.new_usertype<util::time::Duration>("Duration");
    time["now"] = []() -> util::time::Date { return util::time::Now(); };

    // util.log
    sol::table log = util.create("log");
    log["trace"] = [](std::string message) -> void { util::log::Trace("LUA", "{}", message); };
    log["debug"] = [](std::string message) -> void { util::log::Debug("LUA", "{}", message); };
    log["info"] = [](std::string message) -> void { util::log::Info("LUA", "{}", message); };
    log["warn"] = [](std::string message) -> void { util::log::Warn("LUA", "{}", message); };
    log["error"] = [](std::string message) -> void { util::log::Error("LUA", "{}", message); };
    log["critical"] = [](std::string message) -> void { util::log::Critical("LUA", "{}", message); };
}

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

                global.scripts.storage.insert_or_assign(path, (sol::function{ result }).dump());
            }
        }
    } else if (fs::is_regular_file(file)) {
        std::unique_lock lock{ global.scripts.mutex };

        auto path = file.string();
        auto result = global.state.load_file(path);
        if (!result.valid()) {
            throw std::runtime_error{ fmt::format("File \"{}\" is not a valid script", path) };
        }

        global.scripts.storage.insert_or_assign(path, (sol::function{ result }).dump());
    }
}

Context::Context()
  : state()
  , cache()
{
    Initialize(state);
}

void
Context::retrieve(const std::string& path)
{
    std::shared_lock lock{ global.scripts.mutex };
    sol::function loaded = state.load(global.scripts.storage.at(path).as_string_view());
    cache.insert_or_assign(path, loaded);
}

sol::function_result
Context::eval(const std::string& code)
{
    return state.script(code);
}

} // namespace script

} // namespace game
#include "pch.h"

#include "game/script.hpp"
#include "util/log.hpp"
#include "util/time.hpp"

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

// change all '\' to '/'
void
normalize_path_separators(std::string& path)
{
    for (size_t i = 0; i < path.size(); i++) {
        if (path[i] == '\\') {
            path[i] = '/';
        }
    }
}

Context::Context()
  : state()
  , cache()
{
    Initialize(state);
}

void
Context::load_file(const std::string& path)
{
    auto bytecode = this->state.load_file(path);
    if (!bytecode.valid()) {
        throw std::runtime_error{ fmt::format("File \"{}\" is not a valid script", path) };
    }

    this->cache.emplace(path, sol::safe_function{ bytecode });
}

void
Context::load(const std::string& path)
{
    if (!fs::exists(path)) {
        throw std::runtime_error{ fmt::format("File \"{}\" does not exist", path) };
    }
    if (fs::is_regular_file(path)) {
        load_file(path);
    } else if (fs::is_directory(path)) {
        for (auto const& entry : fs::recursive_directory_iterator(path)) {
            if (fs::is_regular_file(entry) && entry.path().extension() == ".lua") {
                auto entry_path = entry.path().string();
                normalize_path_separators(entry_path);
                load_file(entry_path);
            }
        }
    }
}

Context::Script&
Context::get(const std::string& path)
{
    auto script = this->cache.find(path);
    if (script == this->cache.end()) {
        throw std::runtime_error{ fmt::format("Script \"{}\" is not loaded", path) };
    }
    return script->second;
}

sol::safe_function_result
Context::eval(const std::string& code)
{
    return this->state.safe_script(code);
}

} // namespace script

} // namespace game
#ifndef SERVER_GAME_SCRIPT_HPP_
#define SERVER_GAME_SCRIPT_HPP_

// Requirements:
// 1. Independent LUA states
// 2. Easily extend API

#include <string>
#ifndef NDEBUG
#    define SOL_ALL_SAFETIES_ON 1
#endif
#include <sol/sol.hpp>

namespace sol {
class state;
}

namespace game {

namespace script {

// Load a script
void Load(const std::string& path);

// Retrieve a script
// The script must be loaded before it can be retrieved
//
// @throws if not found
sol::function Get(const std::string& path, sol::state& state);

// Load game API and scripts into State
void Initialize(sol::state&);

} // namespace script

} // namespace game

#endif // SERVER_GAME_SCRIPT_HPP_
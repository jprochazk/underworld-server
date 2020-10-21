#ifndef SERVER_GAME_SCRIPT_HPP_
#define SERVER_GAME_SCRIPT_HPP_

#include "sol/forward.hpp"
#include <string>
#ifndef NDEBUG
#    define SOL_EXCEPTIONS_SAFE_PROPAGATION 1
#    define SOL_ALL_SAFETIES_ON 1
#endif
#define SOL_USE_BOOST 1
#include <sol/sol.hpp>

namespace game {

namespace script {

// Load (or reload) one or more scripts from a file/directory
//
// @throws If file not found, or if script is not valid
void Load(const std::string& path);

class Context
{
public:
    Context();

    // Execute a script
    //
    // @throws If script not found, or if script execution throws
    template<typename... Args>
    inline auto
    execute(const std::string& path, Args... args)
    {
        if (auto it = cache.find(path); it != cache.end()) {
            return it->second.call(std::forward<Args>(args)...);
        } else {
            retrieve(path);
            return cache[path].call(std::forward<Args>(args)...);
        }
    }

    sol::safe_function_result eval(const std::string& code);

private:
    void retrieve(const std::string& path);

    sol::state state;
    std::unordered_map<std::string, sol::safe_function> cache;
};

} // namespace script

} // namespace game

#endif // SERVER_GAME_SCRIPT_HPP_
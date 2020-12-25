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

/*
high-level script architecture:

each spell, AI, item, etc. script will implement a set of functions which form an interface
e.g. an AI interface could look like:

```lua

function spawn(self)
    -- do whatever initialization necessary
    self.target = nil
    self.range = 100
    self.spawn = self.position
end

function update(self)
    self.target = self.target or world.findNearestEnemy(self.spawn, self.range)
    if self.target then
        if math.distance(self.position, self.spawn) > self.range then
            self.target = nil
            await(self:pathTo(self.spawn))
        else
            if math.distance(self.position, self.target.position) < self.stats.meleeRange then
                await(self:attack(self.target))
            else
                await(self:pathTo(
                    self.target,
                    self.range - math.distance(self.position, self.spawn)))
            end
        end
    else
        local dest = world.randomPointInRange(self.spawn, self.range)
        await(self:pathTo(dest))
    end
end
```

Some explanations:
    `self`  in this case is an instance of the NPC entity wrapper which
            contains functions for pathing, attacking, casting spells,
            speaking, etc. it also contains info about the entity,
            like stats, position, type, etc
    `world` is a global namespace containing functions
            that help with interacting with other entities
    `math`  is a global namespace containing common scalar, vector
            and matrix math operations
    `await` is just `coroutine.yield`
*/

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
#include "pch.h"

#ifndef SERVER_GAME_SCRIPT_HPP_
#define SERVER_GAME_SCRIPT_HPP_

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

class Context
{
public:
    using Script = sol::safe_function;

    Context();

    // Load (or reload) a single script (or directory???)
    void load(const std::string& path);
    Script& get(const std::string& path);
    sol::safe_function_result eval(const std::string& code);

private:
    void load_file(const std::string& path);

    sol::state state;
    std::unordered_map<std::string, sol::safe_function> cache;
};

} // namespace script

} // namespace game

#endif // SERVER_GAME_SCRIPT_HPP_
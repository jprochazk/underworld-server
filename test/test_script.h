#include <gtest/gtest.h>

#include "game/script.hpp"
#include "util/time.hpp"
#include <string>

TEST(Script, Initialize)
{
    using namespace game;

    sol::state state;
    script::Initialize(state);

    auto result = state.script("return string.format(\"pi = %.4f\", math.pi)");
    EXPECT_EQ(result.get<std::string>(), std::string{ "pi = 3.1416" });
}

TEST(Script, Date)
{
    using namespace game;

    sol::state state;
    script::Initialize(state);

    auto now = util::time::Now();
    util::time::Date result = state.script("return util.time.now()");
    EXPECT_TRUE(result > now);
}

TEST(Script, Load_File)
{
    using namespace game;

    script::Load("scripts/test.lua");

    sol::state state;
    script::Initialize(state);

    auto now = util::time::Now();
    util::time::Date result = script::Get("scripts/test.lua", state)();
    EXPECT_TRUE(result > now);
}

TEST(Script, Load_Directory)
{
    using namespace game;

    script::Load("scripts");

    sol::state state;
    script::Initialize(state);

    auto now = util::time::Now();
    util::time::Date result = script::Get("scripts/test.lua", state)();
    EXPECT_TRUE(result > now);
}
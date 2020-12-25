#include "pch.h"
#include <gtest/gtest.h>

#include "game/script.hpp"
#include "util/time.hpp"

TEST(Script, Load_File)
{
    spdlog::set_level(spdlog::level::trace);
    using namespace game;

    script::Context context;
    context.load("test/scripts/test.lua");
}

TEST(Script, Load_Directory)
{
    spdlog::set_level(spdlog::level::trace);
    using namespace game;

    script::Context context;
    context.load("test/scripts");
}

TEST(Script, Context)
{
    spdlog::set_level(spdlog::level::trace);
    using namespace game;

    script::Context context;
    context.load("test/scripts");

    auto result = context.eval("return string.format(\"pi = %.4f\", math.pi)");
    EXPECT_EQ(result.get<std::string>(), std::string{ "pi = 3.1416" });
}

TEST(Script, Get)
{
    spdlog::set_level(spdlog::level::trace);
    using namespace game;

    // pre-load scripts
    script::Context context;
    context.load("test/scripts");

    // try to execute a script
    auto now = util::time::Now();
    util::time::Date result = context.get("test/scripts/test.lua")();
    EXPECT_TRUE(result > now);

    // and one from a nested directory
    now = util::time::Now();
    result = context.get("test/scripts/test_dir/test.lua")();
    EXPECT_TRUE(result > now);
}

TEST(Script, API_Date)
{
    spdlog::set_level(spdlog::level::trace);
    using namespace game;

    script::Context context;
    context.load("test/scripts");

    auto now = util::time::Now();
    util::time::Date result = context.eval("return util.time.now()");
    EXPECT_TRUE(result > now);
}
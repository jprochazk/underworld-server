#include <gtest/gtest.h>

#include "game/script.hpp"
#include "util/time.hpp"
#include <string>

TEST(Script, Load_File)
{
    using namespace game;

    script::Load("scripts/test.lua");
}

TEST(Script, Load_Directory)
{
    using namespace game;

    script::Load("scripts");
}

TEST(Script, Context)
{
    using namespace game;

    script::Load("scripts");
    script::Context context;

    auto result = context.eval("return string.format(\"pi = %.4f\", math.pi)");
    EXPECT_EQ(result.get<std::string>(), std::string{ "pi = 3.1416" });
}

TEST(Script, Date)
{
    using namespace game;

    script::Load("scripts");
    script::Context context;

    auto now = util::time::Now();
    util::time::Date result = context.eval("return util.time.now()");
    EXPECT_TRUE(result > now);
}

TEST(Script, Execute)
{
    using namespace game;

    // pre-load scripts
    script::Load("scripts");
    script::Context context;

    // try to execute a script
    auto now = util::time::Now();
    util::time::Date result = context.execute("scripts/test.lua");
    EXPECT_TRUE(result > now);

    // and one from a nested directory
    now = util::time::Now();
    result = context.execute("scripts/test_dir/test.lua");
    EXPECT_TRUE(result > now);
}
#define GAME_TEST_ENVIRONMENT
#include <gtest/gtest.h>

#include "test_packet.h"
#include "test_script.h"
#include "test_world.h"

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
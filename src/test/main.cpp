#include "pch.h"

#define GAME_TEST_ENVIRONMENT
#include <gtest/gtest.h>

// clang-format off
#include "test_packet.h"
#include "test_world.h"
#include "test_script.h"
#include "test_db.h"
//#include "test_ppga.h"
// clang-format on

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
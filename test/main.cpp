
#include <gtest/gtest.h>

//#include "endian_test.h"
#include "packet_test.h"

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
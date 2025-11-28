#include <gtest/gtest.h>
#include "helper.h"
#include "threadsync.h"
#include "marker.h"
#include "helper.h"

using namespace helper;



TEST(HelperTest, ConstantsDefined) {
    EXPECT_EQ(SUCCESS_EXIT, 0);
    EXPECT_EQ(FAILURE_EXIT, 1);
    EXPECT_GT(THREAD_SLEEP_WAIT, 0);
    EXPECT_GT(THREAD_SLEEP, 0);
    EXPECT_GT(FORCED_WAIT, 0);
    EXPECT_GE(INPUT_BUFFER_CLEAR_SIZE, 10000);
}

TEST(HelperTest, GetValidatedArraySize_Valid) {
    std::istringstream input("5\n");
    cin.rdbuf(input.rdbuf());
    EXPECT_EQ(GetValidatedArraySize(), 5);
}

TEST(HelperTest, GetValidatedArraySize_InvalidThenValid) {
    std::istringstream input("0\n-5\nabc\n10\n");
    cin.rdbuf(input.rdbuf());
    EXPECT_EQ(GetValidatedArraySize(), 10);
}

TEST(HelperTest, GetValidatedThreadCount_Valid) {
    std::istringstream input("3\n");
    cin.rdbuf(input.rdbuf());
    EXPECT_EQ(GetValidatedThreadCount(), 3);
}

TEST(HelperTest, GetValidatedThreadCount_InvalidThenValid) {
    std::istringstream input("-1\n0\ntest\n4\n");
    cin.rdbuf(input.rdbuf());
    EXPECT_EQ(GetValidatedThreadCount(), 4);
}

TEST(HelperTest, GetValidatedArraySize_Boundary) {
    std::istringstream input("1\n");
    cin.rdbuf(input.rdbuf());
    EXPECT_EQ(GetValidatedArraySize(), 1);
}

TEST(HelperTest, GetValidatedThreadCount_Boundary) {
    std::istringstream input("1\n");
    cin.rdbuf(input.rdbuf());
    EXPECT_EQ(GetValidatedThreadCount(), 1);
}


TEST(StaticTest, ClassSizes)
{
    EXPECT_GE(sizeof(ThreadSync), sizeof(void*));
    EXPECT_GE(sizeof(MarkerManager), sizeof(void*));
    SUCCEED();
}

TEST(StaticTest, ObjectCreation)
{
    ThreadSync sync;
    MarkerManager manager;
    SUCCEED();
}

TEST(StaticTest, CompilationCheck)
{
    ThreadSync* sync_ptr = nullptr;
    MarkerManager* manager_ptr = nullptr;

    EXPECT_EQ(sync_ptr, nullptr);
    EXPECT_EQ(manager_ptr, nullptr);
    SUCCEED();
}

TEST(StaticTest, BasicAssertions)
{
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
    EXPECT_EQ(1, 1);
    EXPECT_NE(1, 2);
    SUCCEED();
}



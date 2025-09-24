#include <gtest/gtest.h>
#include <climits>
#include "min_max.h"
#include "average.h" 

TEST(FindMinMaxTest, CheakNormalCase) {
    std::vector<int> v = { 5, 3, 8, 1, 9 };
    auto [min, max] = find_min_max(v);
    EXPECT_EQ(min, 1);
    EXPECT_EQ(max, 9);
}

TEST(FindMinMaxTest, CheakNegativeNumbers) {
    std::vector<int> v = { -10, -20, -5, -7, -8 };
    auto [min, max] = find_min_max(v);
    EXPECT_EQ(min, -20);
    EXPECT_EQ(max, -5);
}

TEST(FindMinMaxTest, CheakSingleElement) {
    std::vector<int> v = { 52 };
    auto [min, max] = find_min_max(v);
    EXPECT_EQ(min, 52);
    EXPECT_EQ(max, 52);
}

TEST(FindMinMaxTest, CheakEmptyVector) {
    std::vector<int> v;
    auto [min, max] = find_min_max(v);
    EXPECT_EQ(min, INT_MAX);
    EXPECT_EQ(max, INT_MIN);
}
TEST(FindAverageTest, CheakNormalCase) {
    std::vector<int> v = { 2, 4, 6, 8 };
    double avg = find_average(v);
    EXPECT_DOUBLE_EQ(avg, 5.0);
}
TEST(FindAverageTest, CheakSNegativeNumbers) {
    std::vector<int> v = { -2, -4, -7, -8 };
    double avg = find_average(v);
    EXPECT_DOUBLE_EQ(avg, -5.25);
}
TEST(FindAverageTest, CheakEmptyVector) {
    std::vector<int> v;
    double avg = find_average(v);
    EXPECT_DOUBLE_EQ(avg, 0.0);
}
TEST(FindAverageTest, CheakSingleElement) {
    std::vector<int> v = { 100 };
    double avg = find_average(v);
    EXPECT_DOUBLE_EQ(avg, 100.0);
}

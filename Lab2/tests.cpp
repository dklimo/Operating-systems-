#include <gtest/gtest.h>
#include "min_max.h"

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

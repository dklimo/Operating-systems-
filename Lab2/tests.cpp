#include <gtest/gtest.h>
#include <climits>
#include "core.h"

TEST(FindMinMaxTest, CheakNormalCase) {
    vector<int> v = { 5, 3, 8, 1, 9 };
    auto [min, max] = find_min_max(v);
    EXPECT_EQ(min, 1);
    EXPECT_EQ(max, 9);
}

TEST(FindMinMaxTest, CheakNegativeNumbers) {
    vector<int> v = { -10, -20, -5, -7, -8 };
    auto [min, max] = find_min_max(v);
    EXPECT_EQ(min, -20);
    EXPECT_EQ(max, -5);
}

TEST(FindMinMaxTest, CheakSingleElement) {
    vector<int> v = { 52 };
    auto [min, max] = find_min_max(v);
    EXPECT_EQ(min, 52);
    EXPECT_EQ(max, 52);
}

TEST(FindMinMaxTest, CheakEmptyVector) {
    vector<int> v;
    auto [min, max] = find_min_max(v);
    EXPECT_EQ(min, INT_MAX);
    EXPECT_EQ(max, INT_MIN);
}
TEST(FindAverageTest, CheakNormalCase) {
    vector<int> v = { 2, 4, 6, 8 };
    double avg = find_average(v);
    EXPECT_DOUBLE_EQ(avg, 5.0);
}
TEST(FindAverageTest, CheakSNegativeNumbers) {
    vector<int> v = { -2, -4, -7, -8 };
    double avg = find_average(v);
    EXPECT_DOUBLE_EQ(avg, -5.25);
}
TEST(FindAverageTest, CheakEmptyVector) {
    vector<int> v;
    double avg = find_average(v);
    EXPECT_DOUBLE_EQ(avg, 0.0);
}
TEST(FindAverageTest, CheakSingleElement) {
    vector<int> v = { 100 };
    double avg = find_average(v);
    EXPECT_DOUBLE_EQ(avg, 100.0);
}
TEST(CalculationResultsTest, ParameterizedConstructor) {
    CalculationResults results(5, 15, 10.5);
    EXPECT_EQ(results.min_val, 5);
    EXPECT_EQ(results.max_val, 15);
    EXPECT_DOUBLE_EQ(results.average_val, 10.5);
}
TEST(ReplaceMinMaxWithAverageTest, CheckNormalCase) {
    vector<int> v = { 1, 2, 3, 4, 5 };
    CalculationResults results(1, 5, 3.0);
    vector<int> result = replace_min_max_with_average(v, results);

    EXPECT_EQ(result[0], 3); 
    EXPECT_EQ(result[4], 3); 
    EXPECT_EQ(result[1], 2); 
    EXPECT_EQ(result[2], 3); 
    EXPECT_EQ(result[3], 4); 
}
TEST(ReplaceMinMaxWithAverageTest, CheckAllElementsSame) {
    vector<int> v = { 5, 5, 5, 5 };
    CalculationResults results(5, 5, 5.0);
    vector<int> result = replace_min_max_with_average(v, results);

    for (int val : result) {
        EXPECT_EQ(val, 5);
    }
}
TEST(MinMaxThreadTest, CheckNormalCase) {
    vector<int> v = { 3, 1, 4, 1, 5, 9 };
    g_results = CalculationResults();

    DWORD result = min_max_thread(&v);

    EXPECT_EQ(result, 0);
    EXPECT_EQ(g_results.min_val, 1);
    EXPECT_EQ(g_results.max_val, 9);
}
TEST(AverageThreadTest, CheckNormalCase) {
    vector<int> v = { 2, 4, 6, 8 };
    g_results = CalculationResults();

    DWORD result = average_thread(&v);

    EXPECT_EQ(result, 0);
    EXPECT_DOUBLE_EQ(g_results.average_val, 5.0);
}
TEST(IntegrationTest, CheckIntegration) {
    vector<int> v = { 1, 2, 3, 4, 5 };
    g_results = CalculationResults();

    min_max_thread(&v);
    average_thread(&v);

    EXPECT_EQ(g_results.min_val, 1);
    EXPECT_EQ(g_results.max_val, 5);
    EXPECT_DOUBLE_EQ(g_results.average_val, 3.0);


    vector<int> result = replace_min_max_with_average(v, g_results);
    EXPECT_EQ(result[0], 3); 
    EXPECT_EQ(result[4], 3); 
    EXPECT_EQ(result[1], 2); 
    EXPECT_EQ(result[2], 3); 
    EXPECT_EQ(result[3], 4); 
}


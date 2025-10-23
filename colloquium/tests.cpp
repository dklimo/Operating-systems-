#include <gtest/gtest.h>
#include <vector>
#include <stdexcept>
#include "exesizes.h"  

using namespace std;

TEST(FibonacciTest, HandlesZeroCount) {
    vector<int> result = fibo(0);
    EXPECT_TRUE(result.empty());
}

TEST(FibonacciTest, HandlesNegativeCount) {
    EXPECT_THROW(fibo(-5), invalid_argument);
}

TEST(FibonacciTest, HandlesSingleElement) {
    vector<int> result = fibo(1);
    vector<int> expected = { 0 };
    EXPECT_EQ(result, expected);
}

TEST(FibonacciTest, HandlesTwoElements) {
    vector<int> result = fibo(2);
    vector<int> expected = { 0, 1 };
    EXPECT_EQ(result, expected);
}

TEST(FibonacciTest, HandlesMultipleElements) {
    vector<int> result = fibo(6);
    vector<int> expected = { 0, 1, 1, 2, 3, 5 };
    EXPECT_EQ(result, expected);
}

TEST(FibonacciTest, HandlesLargeSequence) {
    vector<int> result = fibo(10);
    vector<int> expected = { 0, 1, 1, 2, 3, 5, 8, 13, 21, 34 };
    EXPECT_EQ(result, expected);
}


TEST(PalindromeTest, HandlesNegativeNumbers) {
    EXPECT_FALSE(palindrome(-121));
    EXPECT_FALSE(palindrome(-1));
}

TEST(PalindromeTest, HandlesZero) {
    EXPECT_TRUE(palindrome(0));
}

TEST(PalindromeTest, HandlesSingleDigit) {
    EXPECT_TRUE(palindrome(5));
    EXPECT_TRUE(palindrome(9));
}

TEST(PalindromeTest, HandlesSimplePalindromes) {
    EXPECT_TRUE(palindrome(121));
    EXPECT_TRUE(palindrome(1221));
    EXPECT_TRUE(palindrome(12321));
}

TEST(PalindromeTest, HandlesNonPalindromes) {
    EXPECT_FALSE(palindrome(123));
    EXPECT_FALSE(palindrome(1234));
    EXPECT_FALSE(palindrome(10));
}

TEST(ReverseTest, HandlesEmptyList) {
    ListN* result = Reverse(nullptr);
    EXPECT_EQ(result, nullptr);
}

TEST(ReverseTest, HandlesSingleElement) {
    ListN* head = new ListN(1);
    ListN* result = Reverse(head);

    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->value, 1);
    EXPECT_EQ(result->next, nullptr);

    cleanupList(result);
}

TEST(ReverseTest, HandlesTwoElements) {
    ListN* head = new ListN(1);
    head->next = new ListN(2);

    ListN* result = Reverse(head);

    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->value, 2);
    EXPECT_NE(result->next, nullptr);
    EXPECT_EQ(result->next->value, 1);
    EXPECT_EQ(result->next->next, nullptr);

    cleanupList(result);
}

TEST(ReverseTest, HandlesMultipleElements) {
    ListN* head = new ListN(1);
    head->next = new ListN(2);
    head->next->next = new ListN(3);
    head->next->next->next = new ListN(4);

    ListN* result = Reverse(head);

    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->value, 4);
    EXPECT_NE(result->next, nullptr);
    EXPECT_EQ(result->next->value, 3);
    EXPECT_NE(result->next->next, nullptr);
    EXPECT_EQ(result->next->next->value, 2);
    EXPECT_NE(result->next->next->next, nullptr);
    EXPECT_EQ(result->next->next->next->value, 1);
    EXPECT_EQ(result->next->next->next->next, nullptr);

    cleanupList(result);
}

TEST(ReverseTest, HandlesLargeList) {
    ListN* head = new ListN(1);
    ListN* current = head;
    for (int i = 2; i <= 5; i++) {
        current->next = new ListN(i);
        current = current->next;
    }

    ListN* result = Reverse(head);

    current = result;
    for (int i = 5; i >= 1; i--) {
        EXPECT_NE(current, nullptr);
        EXPECT_EQ(current->value, i);
        current = current->next;
    }
    EXPECT_EQ(current, nullptr);

    cleanupList(result);
} 

TEST(PrintResultTest, HandlesNormalCase) {
    testing::internal::CaptureStdout();

    ListN* head = new ListN(1);
    head->next = new ListN(2);
    head->next->next = new ListN(3);

    PrintResult(5, 121, head);

    string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.find("Fibonacci numbers: 0 1 1 2 3") != string::npos);
    EXPECT_TRUE(output.find("Number 121 is palindrome: YES") != string::npos);
    EXPECT_TRUE(output.find("Reversed linked list: 3 2 1") != string::npos);
}

TEST(PrintResultTest, HandlesFibonacciError) {
    testing::internal::CaptureStdout();

    ListN* head = new ListN(1);

    PrintResult(-1, 123, head); 

    string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.find("Fibonacci error") != string::npos);
}

TEST(PerformanceTest, LargeFibonacciSequence) {
    EXPECT_NO_THROW({
        vector<int> result = fibo(1000);
        EXPECT_EQ(result.size(), 1000);
        });
}

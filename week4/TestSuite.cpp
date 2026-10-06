#include <gtest/gtest.h>
#include "TimeParser.h"

// Test suite: TimeParserTest

TEST(TimeParserTest, TestCaseCorrectTime) 
{
    char time_test[] = "000000";
    ASSERT_EQ(time_parse(time_test),0);
}

TEST(TimeParserTest2, TestCaseCorrectTime) 
{
    char time_test[] = "235959";
    ASSERT_EQ(time_parse(time_test), 86399);
}

TEST(TimeParserTest3, TestCaseTimeLen) 
{
    char time_test[] = "23595";
    ASSERT_EQ(time_parse(time_test), TIME_LEN_ERROR);
}

TEST(TimeParserTest4, TestCaseTimeLen) 
{
    char time_test[] = "2359590";
    ASSERT_EQ(time_parse(time_test), TIME_LEN_ERROR);
}

TEST(TimeParserTest41, TestCaseTimeLen) 
{
    char time_test[] = "";
    ASSERT_EQ(time_parse(time_test), TIME_LEN_ERROR);
}

TEST(TimeParserTest5, TestCaseIncorrectArray) 
{
    char *time_test = NULL;
    ASSERT_EQ(time_parse(time_test), TIME_ARRAY_ERROR);
}

TEST(TimeParserTest6, TestCaseInCorrectTime) 
{
    char time_test[] = "999999";
    ASSERT_EQ(time_parse(time_test), TIME_VALUE_ERROR);
}

TEST(TimeParserTest7, TestCaseInCorrectTime)
{
    char time_test[] = "-12345";
    ASSERT_EQ(time_parse(time_test), TIME_VALUE_ERROR);
}

TEST(TimeParserTest8, TestCaseInCorrectTime)
{
    char time_test[] = "240000";
    ASSERT_EQ(time_parse(time_test), TIME_VALUE_ERROR);
}

TEST(TimeParserTest9, TestCaseInCorrectTime)
{
    char time_test[] = "236000";
    ASSERT_EQ(time_parse(time_test), TIME_VALUE_ERROR);
}

TEST(TimeParserTest10, TestCaseInCorrectTime)
{
    char time_test[] = "225960";
    ASSERT_EQ(time_parse(time_test), TIME_VALUE_ERROR);
}

TEST(TimeParserTest11, TestCaseInCorrectTime) // Example code does not regonize letters
{
    char time_test[] = "ABCDEF";
    ASSERT_EQ(time_parse(time_test), TIME_VALUE_ERROR);
}


// https://google.github.io/googletest/reference/testing.html
// https://google.github.io/googletest/reference/assertions.html

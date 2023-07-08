#include "GlobalTestFixture.h"

TEST(TestCreateWindow, CreateWindow)
{
    EXPECT_EQ(envPtr->window().width(), 1024);
    EXPECT_EQ(envPtr->window().height(), 768);
}

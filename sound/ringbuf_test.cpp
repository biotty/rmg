#include "ringbuf.hpp"
#include "gtest/gtest.h"

TEST(Basic, Empty)
{
    ringbuf<int> b(3);
    EXPECT_EQ(0u, b.count());
    EXPECT_EQ(0, b.tap(0));
    EXPECT_EQ(0u, b.count());
    EXPECT_EQ(0, b.get());
    EXPECT_EQ(0u, b.count());
}

TEST(Basic, Single)
{
    ringbuf<int> b(3);
    b.put(9);
    EXPECT_EQ(9, b.tap(0));
    EXPECT_EQ(1u, b.count());
    EXPECT_EQ(9, b.get());
    EXPECT_EQ(0u, b.count());
    b.put(9);
    EXPECT_EQ(9, b.get());
    EXPECT_EQ(0u, b.count());
}

TEST(Basic, Order)
{
    ringbuf<int> b(5);
    b.put(7);
    b.put(8);
    b.put(9);
    EXPECT_EQ(9, b.tap(0));
    EXPECT_EQ(8, b.tap(1));
    EXPECT_EQ(7, b.tap(2));
    EXPECT_EQ(7, b.get());
    EXPECT_EQ(8, b.get());
    EXPECT_EQ(9, b.get());
}

TEST(Basic, Full)
{
    ringbuf<int> b(2);
    b.put(8);
    b.put(9);
    EXPECT_EQ(9, b.tap(0));
    EXPECT_EQ(8, b.tap(1));
    EXPECT_EQ(0, b.tap(2));
    EXPECT_EQ(8, b.get());
    EXPECT_EQ(9, b.get());
    EXPECT_EQ(0, b.get());
}

TEST(Basic, Overflow)
{
    ringbuf<int> b(2);
    b.put(7);
    b.put(8);
    b.put(9);
    EXPECT_EQ(8, b.get());
    EXPECT_EQ(9, b.get());
    EXPECT_EQ(0, b.get());
}

TEST(Basic, Cycle)
{
    ringbuf<int> b(2);
    b.put(7);
    EXPECT_EQ(7, b.get());
    b.put(8);
    EXPECT_EQ(8, b.get());
    b.put(9);
    EXPECT_EQ(9, b.get());
    EXPECT_EQ(0, b.get());
    EXPECT_EQ(0u, b.count());
}

TEST(Basic, Fill)
{
    ringbuf<int> b(2);
    b.fill();
    EXPECT_EQ(2u, b.count());
    EXPECT_EQ(0, b.get());
    EXPECT_EQ(0, b.get());
    EXPECT_EQ(0u, b.count());
}

TEST(Basic, Buffer)
{
    ringbuf<int> b(2);
    b.fill();
    EXPECT_EQ(0, b.get());
    b.put(8);
    EXPECT_EQ(0, b.get());
    b.put(9);
    EXPECT_EQ(8, b.get());
    b.put(0);
    EXPECT_EQ(9, b.get());
    b.put(0);
    EXPECT_EQ(2u, b.count());
}

TEST(Basic, BufferChunk)
{
    ringbuf<int> b(3);
    b.fill();
    b.put(8);
    b.put(9);
    EXPECT_EQ(0, b.get());
    EXPECT_EQ(8, b.get());
    EXPECT_EQ(9, b.get());
}


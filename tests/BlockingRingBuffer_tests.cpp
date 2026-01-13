#include <gtest/gtest.h>

#include <BlockingRingBuffer.h>
#include <thread>

namespace
{
    constexpr std::size_t BufferSize = 64;
    template <class T>
    using RingBuffer = blocking::BlockingRingBuffer<T, BufferSize>;
}

TEST(BlockingRingBuffer_Unit, DefaultCtorTest)
{
    [[maybe_unused]] RingBuffer<int> queue;
}

TEST(BlockingRingBuffer_Unit, PushOneElementTest) {
    RingBuffer<int> queue;
    queue.Push(1);
}

TEST(BlockingRingBuffer_Unit, PopEmptyReturnsStdNulloptTest) {
    RingBuffer<int> queue;
    ASSERT_EQ(queue.Pop(), std::nullopt);
}

TEST(BlockingRingBuffer_Unit, PushPopReturnsSameElementTest) {
    RingBuffer<int> queue;
    constexpr int value = 5;
    queue.Push(value);
    ASSERT_EQ(queue.Pop(), value);
}

TEST(BlockingRingBuffer_Unit, MultiplePushPopReturnsSameElementTest) {
    RingBuffer<int> queue;
    constexpr int iterations = 100;
    for (int i = 0; i < iterations; ++i)
    {
        queue.Push(i);
        ASSERT_EQ(queue.Pop(), i);
    }
}

TEST(BlockingRingBuffer_Unit, CannotOverflowTest) {
    RingBuffer<int> buffer;
    for (std::size_t i = 0; i < BufferSize; ++i)
    {
        ASSERT_TRUE(buffer.Push(i));
    }

    ASSERT_FALSE(buffer.Push(0));
}

TEST(BlockingRingBuffer_Stress, ConcurrentPushAndPopReturnsAllElementsTest) {
    constexpr int iterations = 1000000;

    RingBuffer<int> buffer;

    std::thread producer([&buffer]()
    {
       for (int i = 0; i < iterations; ++i)
       {
           while (!buffer.Push(i)) {}
       }
    });

    auto popped = 0;
    std::thread consumer([&buffer, &popped]()
    {
        while (popped < iterations)
        {
            if (buffer.Pop())
            {
                ++popped;
            }
        }
    });

    producer.join();
    consumer.join();

    ASSERT_EQ(popped, iterations);
    ASSERT_EQ(buffer.Pop(), std::nullopt);
}
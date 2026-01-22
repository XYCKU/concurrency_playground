#include <gtest/gtest.h>

#include <MPMCRingBuffer.h>
#include <thread>

namespace
{
    constexpr std::size_t BufferSize = 2ULL << 15;
    template <class T>
    using RingBuffer = lockfree::MPMCRingBuffer<T, BufferSize>;
}

TEST(MPMCRingBuffer_Unit, DefaultCtorTest)
{
    [[maybe_unused]] RingBuffer<int> queue;
}

TEST(MPMCRingBuffer_Unit, PushOneElementTest) {
    RingBuffer<int> queue;
    queue.Push(1);
}

TEST(MPMCRingBuffer_Unit, PopEmptyReturnsStdNulloptTest) {
    RingBuffer<int> queue;
    ASSERT_EQ(queue.Pop(), std::nullopt);
}

TEST(MPMCRingBuffer_Unit, PushPopReturnsSameElementTest) {
    RingBuffer<int> queue;
    constexpr int value = 5;
    queue.Push(value);
    ASSERT_EQ(queue.Pop(), value);
}

TEST(MPMCRingBuffer_Unit, MultiplePushPopReturnsSameElementTest) {
    RingBuffer<int> queue;
    constexpr int iterations = 100;
    for (int i = 0; i < iterations; ++i)
    {
        queue.Push(i);
        ASSERT_EQ(queue.Pop(), i);
    }
}

TEST(MPMCRingBuffer_Unit, CannotOverflowTest) {
    RingBuffer<int> buffer;
    for (std::size_t i = 0; i < BufferSize; ++i)
    {
        ASSERT_TRUE(buffer.Push(i));
    }

    ASSERT_FALSE(buffer.Push(0));
}

TEST(MPMCRingBuffer_Stress, SingleProducerSingleConsumerTest) {
    constexpr int iterations = 1000;

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

TEST(MPMCRingBuffer_Stress, MultipleProducersMultipleConsumersTest) {
    constexpr int iterations = 10000;
    constexpr int producersAmount = 3;
    constexpr int consumersAmount = 2;

    RingBuffer<int> buffer;
    std::atomic<std::size_t> popped = 0;

    {
        std::vector<std::jthread> producers(producersAmount);
        for (auto & producer : producers)
        {
            producer = std::jthread([&buffer]()
            {
               for (int i = 0; i < iterations; ++i)
               {
                   while (!buffer.Push(i)) {}
               }
            });
        }

        std::vector<std::jthread> consumers(consumersAmount);
        for (auto & consumer : consumers)
        {
            consumer = std::jthread([&buffer, &popped]()
            {
                while (popped.load(std::memory_order_relaxed) < iterations * producersAmount)
                {
                    if (buffer.Pop())
                    {
                        popped.fetch_add(1, std::memory_order_relaxed);
                    }
                }
            });
        }
    }

    ASSERT_EQ(popped, iterations * producersAmount);
    ASSERT_EQ(buffer.Pop(), std::nullopt);
}
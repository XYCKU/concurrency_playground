#include <gtest/gtest.h>

#include <MSQueue.h>
#include <thread>

TEST(MSQueue_Unit, DefaultCtorTest)
{
    [[maybe_unused]] lockfree::MSQueue<int> queue;
}

TEST(MSQueue_Unit, PushOneElementTest) {
    lockfree::MSQueue<int> queue;
    queue.Push(1);
}

TEST(MSQueue_Unit, PopEmptyqueueReturnsStdNulloptTest) {
    lockfree::MSQueue<int> queue;
    ASSERT_EQ(queue.Pop(), std::nullopt);
}

TEST(MSQueue_Unit, PushPopqueueReturnsSameElementTest) {
    lockfree::MSQueue<int> queue;
    constexpr int value = 5;
    queue.Push(value);
    ASSERT_EQ(queue.Pop(), value);
}

TEST(MSQueue_Unit, MultiplePushPopqueueReturnsSameElementTest) {
    lockfree::MSQueue<int> queue;
    constexpr int iterations = 100;
    for (int i = 0; i < iterations; ++i)
    {
        queue.Push(i);
        ASSERT_EQ(queue.Pop(), i);
    }
}

TEST(MSQueue_Stress, ConcurrentPushAndPopReturnsAllElements) {
    constexpr int iterations = 1000000;

    lockfree::MSQueue<int> queue;

    std::thread producer([&queue]()
    {
       for (int i = 0; i < iterations; ++i)
       {
           queue.Push(i);
       }
    });

    int popped = 0;
    std::atomic<bool> isFinished = false;
    std::thread consumer([&queue, &popped, &isFinished]()
    {
        while (!isFinished.load(std::memory_order::acquire))
        {
            if (queue.Pop())
            {
                ++popped;
            }
        }
    });

    producer.join();
    isFinished.store(true, std::memory_order::release);
    consumer.join();

    ASSERT_EQ(popped, iterations);
}
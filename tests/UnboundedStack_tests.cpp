#include <gtest/gtest.h>

#include <UnboundedStack.h>
#include <thread>

TEST(UnboundedStack_Unit, DefaultCtorTest)
{
    [[maybe_unused]] lockfree::UnboundedStack<int> stack;
}

TEST(UnboundedStack_Unit, PushOneElementTest) {
    lockfree::UnboundedStack<int> stack;
    stack.Push(1);
}

TEST(UnboundedStack_Unit, PopEmptyStackReturnsStdNulloptTest) {
    lockfree::UnboundedStack<int> stack;
    ASSERT_EQ(stack.Pop(), std::nullopt);
}

TEST(UnboundedStack_Unit, PushPopStackReturnsSameElementTest) {
    lockfree::UnboundedStack<int> stack;
    constexpr int value = 5;
    stack.Push(value);
    ASSERT_EQ(stack.Pop(), value);
}

TEST(UnboundedStack_Unit, MultiplePushPopStackReturnsSameElementTest) {
    lockfree::UnboundedStack<int> stack;
    constexpr int iterations = 100;
    for (int i = 0; i < iterations; ++i)
    {
        stack.Push(i);
        ASSERT_EQ(stack.Pop(), i);
    }
}

TEST(UnboundedStack_Stress, ConcurrentPushAndPopReturnsAllElements) {
    constexpr int iterations = 1000;

    lockfree::UnboundedStack<int> stack;

    std::thread producer([&stack]()
    {
       for (int i = 0; i < iterations; ++i)
       {
           stack.Push(i);
       }
    });

    int popped = 0;
    std::atomic<bool> isFinished = false;
    std::thread consumer([&stack, &popped, &isFinished]()
    {
        while (!isFinished.load(std::memory_order::acquire))
        {
            if (stack.Pop())
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
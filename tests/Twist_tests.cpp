#include <twist/sim.hpp>
#include <twist/test/body/assert.hpp>
#include <thread>

#include <gtest/gtest.h>

#include <MPMCRingBuffer2.h>

namespace
{
    constexpr std::size_t BufferSize = 2ULL << 15;
    template <class T>
    using RingBuffer = lockfree::MPMCRingBuffer3<T, BufferSize>;
}

TEST(MPMCRingBuffer_Stress, MultipleProducersMultipleConsumersTest) {
    twist::sim::sched::RandomScheduler scheduler{{.seed = 42}};
    twist::sim::Simulator simulator{&scheduler};

    auto result = simulator.Run([]()
    {
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
    });

    ASSERT_TRUE(result.Ok());
}
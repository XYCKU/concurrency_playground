#include <iostream>
#include <queue>
#include <thread>

#include <benchmark/benchmark.h>

#include <BlockingRingBuffer.h>
#include <MPMCRingBuffer.h>
#include <SPSCRingBuffer.h>

namespace
{
    constexpr std::size_t BufferSize = 1ULL << 20;
    template <class T>
    using BlockingRingBuffer = blocking::BlockingRingBuffer<T, BufferSize>;
    template <class T>
    using SPSCLFRingBuffer = lockfree::SPSCRingBuffer<T, BufferSize>;
    template <class T>
    using MPMCLFRingBuffer = lockfree::MPMCRingBuffer<T, BufferSize>;
}

template <template <typename...> class RingBuffer>
static void BM_ConcurrentPushPop(benchmark::State& state) {
    const auto amountPerThread = static_cast<std::size_t>(state.range(0));
    const auto producersAmount = state.range(1);
    const auto consumersAmount = state.range(2);

    const auto prePushedAmount = std::min(amountPerThread / 100 * producersAmount, BufferSize / 2);
    const auto totalAmount = amountPerThread * producersAmount + prePushedAmount;

    RingBuffer<int> buffer;
    benchmark::DoNotOptimize(buffer);

    for (auto _ : state)
    {
        state.PauseTiming();
        std::vector<std::jthread> producers(producersAmount);
        std::vector<std::jthread> consumers(consumersAmount);
        state.ResumeTiming();

        for (std::size_t i = 0; i < prePushedAmount; ++i)
        {
            buffer.Push(i);
        }

        std::atomic<std::size_t> popped = 0;

        for (auto & producer : producers)
        {
            producer = std::jthread([&buffer, amountPerThread]()
            {
               for (std::size_t i = 0; i < amountPerThread; ++i)
               {
                   while (!buffer.Push(i)) {}
                   benchmark::ClobberMemory();
               }
            });
        }

        for (auto & consumer : consumers)
        {
            consumer = std::jthread([&buffer, &popped, totalAmount]()
            {
                while (popped.load(std::memory_order_relaxed) < totalAmount)
                {
                    if (buffer.Pop())
                    {
                        popped.fetch_add(1, std::memory_order_relaxed);
                        benchmark::ClobberMemory();
                    }
                }
            });
        }

        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_ConcurrentPushPop<BlockingRingBuffer>)->ArgsProduct(
{
    //benchmark::CreateRange(10, 100'000, 100),
    {1'000'000},
    benchmark::CreateDenseRange(1, 3, 2),
    benchmark::CreateDenseRange(1, 3, 2)
});

BENCHMARK(BM_ConcurrentPushPop<SPSCLFRingBuffer>)->ArgsProduct(
{
    //benchmark::CreateRange(1, 1'000'000, 10),
    {1'000'000},
    benchmark::CreateDenseRange(1, 1, 1),
    benchmark::CreateDenseRange(1, 1, 1)}
);

BENCHMARK(BM_ConcurrentPushPop<MPMCLFRingBuffer>)->ArgsProduct(
{
    //benchmark::CreateRange(1, 1'000'000, 10),
    {1'000'000},
    benchmark::CreateDenseRange(1, 3, 2),
    benchmark::CreateDenseRange(1, 3, 2)
});

BENCHMARK_MAIN();

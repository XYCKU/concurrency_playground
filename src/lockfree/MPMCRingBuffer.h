#pragma once

#include <atomic>
#include <cstddef>
#include <optional>

#include <Alignment.h>
#include <Math.h>

namespace lockfree
{
    template <class T, std::size_t Size>
    class MPMCRingBuffer
    {
        static_assert(math::IsPowerOf2(Size), "Size must be a power of 2");

        struct alignas(alignment::hardware_destructive_interference_size) Cell
        {
            T data;
            std::atomic<bool> ready = false;
        };

    public:
        bool Push(T data)
        {
            auto tail = tail_.load(std::memory_order_relaxed);

            while (true)
            {
                if (tail - head_.load(std::memory_order_relaxed) == Size)
                {
                    return false;
                }

                auto& cell = data_[Index(tail)];
                if (cell.ready.load(std::memory_order_acquire))
                {
                    tail = tail_.load(std::memory_order_relaxed);
                    continue;
                }

                if (tail_.compare_exchange_weak(tail, tail + 1, std::memory_order_relaxed, std::memory_order_relaxed))
                {
                    cell.data = std::move(data);
                    cell.ready.store(true, std::memory_order_release);
                    return true;
                }
            }
        }

        std::optional<T> Pop()
        {
            while (true)
            {
                auto head = head_.load(std::memory_order_acquire);
                if (tail_.load(std::memory_order_relaxed) - head == 0)
                {
                    return std::nullopt;
                }

                auto& cell = data_[Index(head)];
                if (!cell.ready.load(std::memory_order_acquire))
                {
                    continue;
                }

                if (!head_.compare_exchange_weak(head, head + 1, std::memory_order_release, std::memory_order_relaxed))
                {
                    continue;
                }

                auto data = std::move(cell.data);
                cell.ready.store(false, std::memory_order_release);
                return std::move(data);
            }
        }

    private:
        static constexpr std::size_t Index(std::size_t index)
        {
            return index & (Size - 1);
        }

    private:
        alignas(alignment::hardware_destructive_interference_size) std::atomic<std::size_t> head_{0};
        alignas(alignment::hardware_destructive_interference_size) std::atomic<std::size_t> tail_{0};
        std::vector<Cell> data_ = std::vector<Cell>(Size);
    };

}

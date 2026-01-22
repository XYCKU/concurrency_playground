#pragma once

#include <atomic>
#include <cstddef>
#include <optional>
#include <vector>

#include <Alignment.h>
#include <Math.h>

namespace lockfree
{
    template <class T, std::size_t Capacity>
    class SPSCRingBuffer
    {
        static_assert(math::IsPowerOf2(Capacity), "Size must be a power of 2");

    public:
        bool Push(T data)
        {
            auto tail = tail_.load(std::memory_order_relaxed);
            if (tail - head_cached_ == Capacity)
            {
                head_cached_ = head_.load(std::memory_order_acquire);
                if (tail - head_cached_ == Capacity)
                {
                    return false;
                }
            }

            data_[Index(tail)] = std::move(data);
            tail_.store(tail + 1, std::memory_order_release);
            return true;
        }

        std::optional<T> Pop()
        {
            auto head = head_.load(std::memory_order_relaxed);
            if (head == tail_cached_)
            {
                tail_cached_ = tail_.load(std::memory_order_acquire);
                if (head == tail_cached_)
                {
                    return std::nullopt;
                }
            }

            auto data = std::move(data_[Index(head)]);
            head_.store(head + 1, std::memory_order_release);
            return data;
        }

    private:
        static constexpr std::size_t Index(std::size_t index)
        {
            return index & (Capacity - 1);
        }

    private:
        alignas(alignment::hardware_destructive_interference_size) std::atomic<std::size_t> head_{0};
        alignas(alignment::hardware_destructive_interference_size) std::size_t head_cached_{0};
        alignas(alignment::hardware_destructive_interference_size) std::atomic<std::size_t> tail_{0};
        alignas(alignment::hardware_destructive_interference_size) std::size_t tail_cached_{0};
        std::vector<T> data_ = std::vector<T>(Capacity);
    };

}

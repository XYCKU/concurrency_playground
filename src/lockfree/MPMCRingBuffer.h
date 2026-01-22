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
    class MPMCRingBuffer
    {
        static_assert(Capacity > 1, "Capacity is too small!");
        static_assert(math::IsPowerOf2(Capacity), "Capacity must be a power of 2!");

        struct Cell
        {
            T data = {};
            std::atomic<uint64_t> sequence;
        };

    public:
        MPMCRingBuffer()
        {
            for (std::size_t i = 0; i < data_.size(); ++i)
            {
                data_[i].sequence.store(i, std::memory_order_relaxed);
            }
        }

        bool Push(T data)
        {
            while (true)
            {
                auto pos = enqueue_pos_.load(std::memory_order_relaxed);
                auto& cell = data_[Index(pos)];
                auto sequence = cell.sequence.load(std::memory_order_acquire);

                const auto dif = static_cast<int64_t>(sequence) - static_cast<int64_t>(pos);
                if (dif < 0)
                {
                    return false;
                }

                if (dif != 0)
                {
                    continue;
                }

                if (enqueue_pos_.compare_exchange_strong(pos, pos + 1, std::memory_order_relaxed))
                {
                    cell.data = std::move(data);
                    cell.sequence.store(sequence + 1, std::memory_order_release);
                    return true;
                }
            }
        }

        std::optional<T> Pop()
        {
            while (true)
            {
                auto pos = dequeue_pos_.load(std::memory_order_relaxed);
                auto& cell = data_[Index(pos)];
                auto sequence = cell.sequence.load(std::memory_order_acquire);

                auto dif = static_cast<int64_t>(sequence) - static_cast<int64_t>(pos + 1);
                if (dif < 0)
                {
                    return std::nullopt;
                }

                if (dif != 0)
                {
                    continue;
                }

                if (dequeue_pos_.compare_exchange_strong(pos, pos + 1, std::memory_order_relaxed))
                {
                    auto result = std::make_optional(std::move(cell.data));
                    cell.sequence.store(pos + Capacity, std::memory_order_release);
                    return result;
                }
            }
        }

    private:
        static constexpr std::size_t Index(std::size_t index)
        {
            return index & (Capacity - 1);
        }

    private:
        alignas(alignment::hardware_destructive_interference_size) std::atomic<std::size_t> dequeue_pos_{0};
        alignas(alignment::hardware_destructive_interference_size) std::atomic<std::size_t> enqueue_pos_{0};
        std::vector<Cell> data_ = std::vector<Cell>(Capacity);
    };
}

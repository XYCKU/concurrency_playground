#pragma once

#include <cstddef>
#include <mutex>
#include <optional>
#include <utility>

namespace blocking
{
    namespace detail
    {
        constexpr bool IsPowerOf2(std::size_t num)
        {
            return (num & (num - 1)) == 0;
        }
    }

    template <class T, std::size_t Size>
    class BlockingRingBuffer
    {
        static_assert(detail::IsPowerOf2(Size), "Size must be a power of 2");

    public:
        bool Push(T data)
        {
            std::lock_guard guard(mutex_);
            if (tail_ - head_ == Size)
            {
                return false;
            }

            data_[Index(tail_++)] = std::move(data);
            return true;
        }

        std::optional<T> Pop()
        {
            std::lock_guard guard(mutex_);
            if (head_ == tail_)
            {
                return std::nullopt;
            }

            return std::move(data_[Index(head_++)]);
        }

    private:
        static constexpr std::size_t Index(std::size_t index)
        {
            return index & (Size - 1);
        }

    private:
        std::mutex mutex_;
        std::size_t head_{0};
        std::size_t tail_{0};
        std::vector<T> data_ = std::vector<T>(Size);
    };
}
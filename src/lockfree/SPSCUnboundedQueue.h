#pragma once

#include <atomic>
#include <optional>

namespace lockfree
{
    template <class T>
    class SPSCUnboundedQueue
    {
        struct Node
        {
            T value{};
            std::atomic<Node*> next = nullptr;
        };

    public:
        SPSCUnboundedQueue()
        {
            auto* dummy = new Node{};
            head_.store(dummy, std::memory_order_relaxed);
            tail_.store(dummy, std::memory_order_relaxed);
        }

        ~SPSCUnboundedQueue()
        {
            while (head_.load(std::memory_order_relaxed))
            {
                delete head_.exchange(head_.load()->next, std::memory_order_relaxed);
            }
        }

        void Push(T data)
        {
            auto* node = new Node{ .value = std::move(data) };
            while (true)
            {
                auto tail = tail_.load(std::memory_order_relaxed);
                Node* null_ptr = nullptr;
                if (!tail->next.compare_exchange_weak(null_ptr, node, std::memory_order_release, std::memory_order_relaxed))
                {
                    continue;
                }

                tail_.compare_exchange_strong(tail, node, std::memory_order_release, std::memory_order_relaxed);
                break;
            }
        }

        std::optional<T> Pop()
        {

        }


    private:
        std::atomic<Node*> head_ = nullptr;
        std::atomic<Node*> tail_ = nullptr;
    };
}
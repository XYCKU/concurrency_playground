#pragma once
#include <atomic>
#include <optional>

namespace lockfree
{
    template <class T>
    class MSQueue
    {
        struct Node
        {
            T value;
            std::atomic<Node*> next = nullptr;
        };

    public:
        MSQueue()
        {
            m_head = m_tail = new Node{};
        }

        ~MSQueue()
        {
            while (m_head)
            {
                delete m_head.exchange(m_head.load()->next);
            }
        }


        void Push(T value)
        {
            Node* newTail = new Node{ .value = std::move(value) };
            Node* currentTail;

            while (true)
            {
                currentTail = m_tail.load();
                if (currentTail->next.load() != nullptr)
                {
                    m_tail.compare_exchange_weak(currentTail, currentTail->next.load());
                    continue;
                }

                Node* empty = nullptr;
                if (currentTail->next.compare_exchange_weak(empty, newTail))
                    break;
            }

            m_tail.compare_exchange_strong(currentTail, newTail);
        }

        std::optional<T> Pop()
        {
            while (true)
            {
                auto* currentHead = m_head.load();
                if (currentHead->next.load() == nullptr)
                {
                    return std::nullopt;
                }

                Node* currentTail = m_tail.load();
                if (currentHead == currentTail)
                {
                    m_tail.compare_exchange_weak(currentTail, currentTail->next.load());
                }

                if (m_head.compare_exchange_weak(currentHead, currentHead->next))
                {
                    Node* next = currentHead->next;
                    T value = std::move(next->value);
                    //delete current;
                    return std::move(value);
                }
            }
        }


    private:
        std::atomic<Node*> m_head = nullptr;
        std::atomic<Node*> m_tail = nullptr;
    };
}
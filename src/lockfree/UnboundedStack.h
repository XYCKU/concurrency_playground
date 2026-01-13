#pragma once

#include <atomic>
#include <optional>

#include "../utils/Packing.h"

namespace lockfree
{
    template <class T>
    class UnboundedStack
    {
        struct Node
        {
            T value{};
            Node* next = nullptr;
        };

        struct TaggedPtr
        {
            Node* ptr = nullptr;
            uint16_t tag = 0;
        };

    public:
        UnboundedStack() = default;

        ~UnboundedStack()
        {
            Node* ptr = Unpack(m_head.load(std::memory_order_relaxed)).ptr;
            while (ptr)
            {
                Node* next = ptr->next;
                delete ptr;
                ptr = next;
            }
        }

        void Push(T value);
        std::optional<T> Pop();

    private:
        static uint64_t Pack(TaggedPtr node);
        static TaggedPtr Unpack(uint64_t data);

    private:
        std::atomic<uint64_t> m_head{};
    };

    template<class T>
    void UnboundedStack<T>::Push(T value)
    {
        auto* newNode = new Node{ .value = std::move(value) };

        uint64_t oldHeadData;
        TaggedPtr newHead { .ptr = newNode };

        do
        {
            oldHeadData = m_head.load(std::memory_order::acquire);
            auto oldHead = Unpack(oldHeadData);
            newNode->next = oldHead.ptr;
            newHead.tag = oldHead.tag + 1;
        }
        while (!m_head.compare_exchange_weak(oldHeadData, Pack(newHead), std::memory_order::release, std::memory_order::relaxed));
    }

    template<class T>
    std::optional<T> UnboundedStack<T>::Pop()
    {
        uint64_t oldHeadData;
        TaggedPtr oldHead;
        TaggedPtr newHead;

        do
        {
            oldHeadData = m_head.load(std::memory_order::acquire);
            oldHead = Unpack(oldHeadData);
            if (!oldHead.ptr)
            {
                return std::nullopt;
            }

            newHead.ptr = oldHead.ptr->next;
            newHead.tag = oldHead.tag + 1;
        } while (m_head.compare_exchange_weak(oldHeadData, Pack(newHead), std::memory_order::release, std::memory_order::relaxed));

        auto result = std::move(oldHead.ptr->value);
        delete oldHead.ptr;
        return std::move(result);
    }

    template<class T>
    uint64_t UnboundedStack<T>::Pack(TaggedPtr node)
    {
       return packing::PackPointerWithData(node.ptr, node.tag);
    }

    template<class T>
    typename UnboundedStack<T>::TaggedPtr UnboundedStack<T>::Unpack(uint64_t data)
    {
        return {
            .ptr = packing::UnpackPointer<Node>(data),
            .tag = packing::UnpackData(data)
        };
    }
}

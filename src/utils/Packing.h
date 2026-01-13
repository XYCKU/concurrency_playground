#pragma once
#include <cstdint>

namespace packing
{
    namespace detail
    {
        constexpr auto kPointerShift = 48;
        constexpr auto kPointerMask = (1ull << kPointerShift) - 1;
    }

    uint64_t PackPointer(void* ptr)
    {
        return reinterpret_cast<uint64_t>(ptr) & detail::kPointerMask;
    }

    uint64_t PackPointerWithData(void* ptr, uint16_t data)
    {
        return (static_cast<uint64_t>(data) << detail::kPointerShift) | PackPointer(ptr);
    }

    void* UnpackPointer(uint64_t data)
    {
        return reinterpret_cast<void*>(data & detail::kPointerMask);
    }

    template <class TPtr>
    constexpr TPtr* UnpackPointer(uint64_t data)
    {
        return static_cast<TPtr*>(UnpackPointer(data));
    }

    constexpr uint16_t UnpackData(uint64_t data)
    {
        return static_cast<uint16_t>(data >> detail::kPointerShift);
    }
}

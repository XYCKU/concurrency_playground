#pragma once

#include <cstddef>

namespace math
{
    constexpr bool IsPowerOf2(std::size_t num)
    {
        return (num & (num - 1)) == 0;
    }
}
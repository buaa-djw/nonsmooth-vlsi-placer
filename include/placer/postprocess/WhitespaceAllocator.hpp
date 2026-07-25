#pragma once
#include "placer/multilevel/Level.hpp"
namespace placer
{
    struct WhitespaceAllocationResult
    {
        size_t objects{};
        size_t leaves{};
        double rms_displacement{};
    };
    [[nodiscard]] WhitespaceAllocationResult allocateWhitespace(Level &level, const Region &region, double target_density, int leaf_size, double min_fraction);
}

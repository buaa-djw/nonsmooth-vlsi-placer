#pragma once
#include "placer/multilevel/Level.hpp"
namespace placer
{
    struct MacroShiftStats
    {
        size_t macros{}, moved{}, failed{};
        double total_displacement{};
    };
    [[nodiscard]] MacroShiftStats macroShifting(Level &, const Region &, int rings, double gap);
}

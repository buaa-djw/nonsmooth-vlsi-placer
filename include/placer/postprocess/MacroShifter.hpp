#pragma once
#include "placer/Common.hpp"
#include "placer/multilevel/Level.hpp"
namespace placer
{
struct MacroShiftStats { std::size_t macros{}, moved{}, failed{}; double total_displacement{}; };
[[nodiscard]] MacroShiftStats macroShifting(Level &, const Region &, double row_height, int rings, double gap);
}

#pragma once
#include <cstddef>
namespace placer { constexpr double EPS = 1.0e-12; using CellId=std::size_t; using PinId=std::size_t; using NetId=std::size_t; struct Region{double xl{0},xh{1},yl{0},yh{1};}; }

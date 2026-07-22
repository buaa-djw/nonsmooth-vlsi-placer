#pragma once
#include "placer/multilevel/Level.hpp"
namespace placer { [[nodiscard]] bool needsNullspaceSeed(const Level&); void seedGrid(Level&, const Region&, int seed); void projectLevel(Level&, const Region&); void projectObject(LObject&, const Region&); }

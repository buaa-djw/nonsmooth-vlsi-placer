#pragma once
#include "placer/multilevel/Level.hpp"
namespace placer
{
    [[nodiscard]] bool needsNullspaceSeed(const Level &);
    void seedGrid(Level &, const Region &, int seed);
    struct ProjectionStats { int projected_objects = 0; double max_shift = 0.0; };
    [[nodiscard]] ProjectionStats projectLevel(Level &, const Region &);
    [[nodiscard]] ProjectionStats projectObject(LObject &, const Region &);
}

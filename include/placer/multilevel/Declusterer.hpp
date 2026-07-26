#pragma once
#include "placer/multilevel/Level.hpp"
namespace placer
{
    struct DeclusterStats
    {
        size_t parents{}, shifted_parents{}, impossible_groups{};
        size_t inherited_children{}, fixed_children_unchanged{};
        double max_parent_shift{};
    };
    [[nodiscard]] DeclusterStats decluster(Level &fine, Level &coarse, const Region &);
}

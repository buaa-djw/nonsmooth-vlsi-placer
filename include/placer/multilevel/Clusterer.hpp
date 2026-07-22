#pragma once
#include "placer/multilevel/Level.hpp"
namespace placer
{
    struct ClusterConfig
    {
        int current{150};
        double coarsen_ratio{5.0};
        int max_levels{16};
        int degree_cap{256};
    };
    [[nodiscard]] Level clusterOneLevel(const Level &, const ClusterConfig &);
    [[nodiscard]] Level clusterOneLevel(const Level &, std::size_t target_movable, int degree_cap);
    [[nodiscard]] std::vector<Level> buildHierarchy(const Level &, const ClusterConfig &);
}

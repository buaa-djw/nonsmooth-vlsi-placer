#pragma once
#include "placer/Config.hpp"
#include "placer/multilevel/Level.hpp"
namespace placer
{
    struct WireEval
    {
        double hpwl{};
        std::vector<double> gx, gy;
    };
    struct InterlevelHpwl
    {
        double coarse_hpwl{}, fine_hpwl{}, delta{}, relative_delta{}, ratio{};
        size_t coarse_nets{}, fine_nets{}, paired_nets{};
        double sum_abs_net_delta{}, max_abs_net_delta{};
    };
    [[nodiscard]] double netHpwl(const Level &, const LNet &);
    [[nodiscard]] double exactHpwl(const Level &);
    [[nodiscard]] WireEval wirelengthSubgradient(const Level &, WirelengthMode mode = WirelengthMode::PaperL1);
    [[nodiscard]] InterlevelHpwl interlevelHpwlConsistency(const Level &, const Level &);
}

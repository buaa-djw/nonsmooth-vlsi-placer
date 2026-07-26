#pragma once
#include "placer/Config.hpp"
#include "placer/multilevel/Level.hpp"
#include <cstdint>
#include <random>
namespace placer
{
    struct WireEval
    {
        double hpwl{}, paper_l1_value{};
        std::vector<double> gx, gy;
        std::size_t effective_edge_count{}, effective_edge_count_x{}, effective_edge_count_y{}, tie_count{};
        double max_l1_hpwl_difference{};
    };
    struct InterlevelHpwl
    {
        double coarse_hpwl{}, fine_hpwl{}, delta{}, relative_delta{}, ratio{};
        size_t coarse_nets{}, fine_nets{}, paired_nets{}, fine_only_nets{}, coarse_only_nets{};
        double sum_abs_net_delta{}, max_abs_net_delta{};
        double internalized_fine_hpwl{}, coarse_only_hpwl{}, matched_coarse_hpwl{}, matched_fine_hpwl{};
    };
    [[nodiscard]] double netHpwl(const Level &, const LNet &);
    [[nodiscard]] double exactHpwl(const Level &);
    class WirelengthEvaluator
    {
    public:
        explicit WirelengthEvaluator(std::uint64_t seed = 1U);
        [[nodiscard]] WireEval evaluate(const Level &, WirelengthMode mode = WirelengthMode::PaperL1, bool update_state = true);
        void reset(std::uint64_t seed);
    private:
        std::mt19937_64 random_;
        int level_index_{};
        bool has_previous_{};
        std::vector<std::vector<double>> previous_x_, previous_y_;
    };
    [[nodiscard]] WireEval evaluateWirelength(const Level &, WirelengthMode mode = WirelengthMode::PaperL1);
    [[nodiscard]] WireEval wirelengthSubgradient(const Level &, WirelengthMode mode = WirelengthMode::PaperL1);
    [[nodiscard]] InterlevelHpwl interlevelHpwlConsistency(const Level &, const Level &);
}

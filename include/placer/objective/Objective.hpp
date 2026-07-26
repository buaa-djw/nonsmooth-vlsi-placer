#pragma once
#include "placer/objective/Density.hpp"
#include "placer/objective/Wirelength.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace placer
{
    enum class ObjectiveEvaluationMode { ValueOnly, StatefulGradient };
    struct ObjectiveEvaluation
    {
        double exact_hpwl{}, paper_l1_value{}, density_penalty{}, paper_ofr{}, lambda{}, objective{};
        double total_overflow{}, maximum_bin_overflow{}, maximum_bin_density{};
        std::vector<double> wire_grad_x, wire_grad_y, density_grad_x, density_grad_y, grad_x, grad_y;
        double wire_grad_l1{}, wire_grad_l2{}, density_grad_l1{}, density_grad_l2{}, combined_grad_l1{}, combined_grad_l2{};
        std::size_t wire_tie_count{}, overflow_bin_count{};
    };
    struct ObjectiveCallCounts
    {
        std::size_t value_evaluations{}, stateful_evaluations{}, wire_state_commits{}, density_evaluations{};
    };
    class ObjectiveEvaluator
    {
    public:
        explicit ObjectiveEvaluator(std::uint64_t wire_seed = 1U);
        [[nodiscard]] ObjectiveEvaluation evaluate(const Level &, const DensityGrid &, double lambda,
                                                   ObjectiveEvaluationMode mode,
                                                   WirelengthMode wire_mode = WirelengthMode::PaperL1);
        void reset(std::uint64_t wire_seed);
        [[nodiscard]] const ObjectiveCallCounts &callCounts() const { return calls_; }
    private:
        WirelengthEvaluator wire_;
        ObjectiveCallCounts calls_;
    };
}

#pragma once
#include "placer/Config.hpp"
#include "placer/objective/Density.hpp"
#include "placer/objective/Objective.hpp"
#include "placer/objective/Wirelength.hpp"
#include <chrono>
#include <string>
namespace placer
{
    struct HistoryRow
    {
        int global_iteration{}, level{}, stage{}, iteration{};
        double lambda{}, raw_hpwl{}, raw_density_penalty{}, raw_objective{}, ofr{};
        double wire_gradient_l1{}, density_gradient_l1{}, weighted_density_gradient_l1{};
        double total_gradient_l1{}, total_gradient_l2{}, beta_pr{}, direction_norm{};
        double s{}, bin_width{}, alpha{}, displacement_norm{};
        int overflow_bin_count{};
        double total_overflow{}, max_bin_overflow{}, max_bin_utilization{};
        double stage_best_objective{};
        int stage_best_iteration{}, no_improve_count{}, projection_count{};
        bool is_stage_best{}, restart_due_to_zero_previous_norm{};
        double elapsed_sec{};
    };
    struct StageResult
    {
        int stage{}, iterations{}, best_iteration{};
        double lambda{}, hpwl{}, density_penalty{}, objective{}, ofr{};
        std::string stop_reason;
    };
    struct OptimizeConfig
    {
        WirelengthMode mode{WirelengthMode::PaperL1};
        int iterations_per_stage{10000}, penalty_stages{20}, nmax{0}, level_index{};
        bool density_only{false};
        double lambda0{}, target_ofr{};
        double s0{0.2}, s_floor{0.06};
        double delta1{1.6}, delta2{1.9}, delta3{2.2};
        int report_every{10};
        std::uint64_t wire_seed{1U};
    };
    struct GlobalOptimizeState { int iteration{}; std::chrono::steady_clock::time_point start_time{}; };
    struct OptimizeResult
    {
        double initial_hpwl{}, initial_density_penalty{}, initial_ofr{};
        double hpwl{}, density_penalty{}, ofr_report{}, lambda{};
        double wire_gradient_l1{}, density_gradient_l1{}, initial_lambda{};
        std::string stop_reason;
        std::vector<StageResult> stages;
        std::vector<HistoryRow> history;
    };
    [[nodiscard]] double paperStepScale(int iteration);
    [[nodiscard]] double paperStepScale(int iteration, double s0, double floor);
    [[nodiscard]] double paperInitialLambda(double wire_gradient_l1, double density_gradient_l1);
    [[nodiscard]] double polakRibiereBeta(const std::vector<double>& g, const std::vector<double>& previous);
    [[nodiscard]] double nextPaperLambda(double lambda, double current_ofr, double previous_ofr, bool has_previous);
    [[nodiscard]] double nextPaperLambda(double lambda, double current_ofr, double previous_ofr, bool has_previous,
                                         double delta1, double delta2, double delta3);
    [[nodiscard]] int paperNoImprovementLimit(size_t movable_count);
    [[nodiscard]] OptimizeResult optimizeLevel(Level &, const Region &, const DensityGrid &, const OptimizeConfig &, GlobalOptimizeState &);
}

#pragma once
#include "placer/multilevel/Level.hpp"
#include <vector>
namespace placer
{
    struct DensityConfig
    {
        Region region;
        int bins_x{}, bins_y{};
        double target_density{};
    };
    struct Overlap1D
    {
        double length{}, translation_subgradient{};
    };
    struct RectangleOverlap
    {
        Overlap1D x, y;
        double area{};
    };
    [[nodiscard]] Overlap1D exactOverlap1D(double object_low, double object_size, double bin_low, double bin_high);
    [[nodiscard]] RectangleOverlap exactRectangleOverlap(double object_x, double object_y, double object_width, double object_height,
                                                         double bin_x0, double bin_x1, double bin_y0, double bin_y1);
    struct DensityMetrics
    {
        double total_object_area{}, total_clipped_area{}, total_bin_overlap{};
        double total_movable_area{}, total_fixed_area{};
        double total_overflow{}, total_positive_overflow{}, quadratic_penalty{}, paper_ofr{};
        std::size_t overflow_bin_count{};
        double maximum_bin_density{}, maximum_raw_overflow{}, maximum_positive_overflow{}, max_bin_overflow{};
        std::size_t candidate_pair_count{}, positive_overlap_pair_count{}, touching_pair_count{}, gradient_contribution_count{};
    };
    struct DensityEval : DensityMetrics
    {
        // Compatibility aliases. Optimization and reporting both use paper_ofr.
        double penalty{}, ofr_penalty{}, ofr_report{}, max_density{};
        int overflow_bins_penalty{}, overflow_bins_report{};
        std::vector<double> gx, gy;
        std::vector<double> bin_density_area, bin_capacity, raw_overflow, positive_overflow, squared_overflow;
    };
    class DensityGrid
    {
    public:
        explicit DensityGrid(DensityConfig config);
        DensityGrid(Region region, int nx, int ny, double target_density);
        // Legacy source-compatible form: two densities are accepted only when equal.
        DensityGrid(Region region, int nx, int ny, double penalty_density, double report_density, unsigned seed = 1);
        [[nodiscard]] DensityEval evaluate(const Level &) const;
        [[nodiscard]] int nx() const { return config_.bins_x; }
        [[nodiscard]] int ny() const { return config_.bins_y; }
        [[nodiscard]] double binW() const { return bin_width_; }
        [[nodiscard]] double binH() const { return bin_height_; }
        [[nodiscard]] double targetDensity() const { return config_.target_density; }
        [[nodiscard]] const DensityConfig &config() const { return config_; }
    private:
        DensityConfig config_;
        double bin_width_{}, bin_height_{};
    };
}

#pragma once
#include "placer/multilevel/Level.hpp"
#include <random>
namespace placer
{
    struct DensityMetrics
    {
        double total_object_area{}, total_clipped_area{};
        double total_movable_area{}, total_fixed_area{};
        double total_overflow{}, quadratic_penalty{}, paper_ofr{};
        std::size_t overflow_bin_count{};
        double maximum_bin_density{}, max_bin_overflow{};
    };
    struct DensityEval : DensityMetrics
    {
        // Compatibility aliases. Both optimization and reporting use paper_ofr.
        double penalty{}, ofr_penalty{}, ofr_report{}, max_density{};
        int overflow_bins_penalty{}, overflow_bins_report{};
        std::vector<double> gx, gy;
    };
    class DensityGrid
    {
    public:
        DensityGrid(Region r, int nx, int ny, double pd, double rd, unsigned seed = 1);
        [[nodiscard]] DensityEval evaluate(const Level &) const;
        [[nodiscard]] int nx() const { return nx_; }
        [[nodiscard]] int ny() const { return ny_; }
        [[nodiscard]] double binW() const { return bw_; }
        [[nodiscard]] double binH() const { return bh_; }

    private:
        Region r_;
        int nx_, ny_;
        double pd_, rd_, bw_, bh_;
        mutable std::mt19937_64 random_;
    };
}

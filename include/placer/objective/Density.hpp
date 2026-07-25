#pragma once
#include "placer/multilevel/Level.hpp"
namespace placer
{
    struct DensityEval
    {
        double penalty = 0.0;
        // Paper Equation (18).
        double ofr = 0.0;
        double total_overflow = 0.0;
        double max_bin_overflow = 0.0;
        double max_density = 0.0;
        double total_movable_area = 0.0;
        double total_fixed_area = 0.0;
        double total_object_area = 0.0;
        int overflow_bin_count = 0;
        std::vector<double> gx, gy;
    };
    class DensityGrid
    {
    public:
        DensityGrid(Region r, int nx, int ny, double target_density);
        // Compatibility overload: paper mode requires equal densities.
        DensityGrid(Region r, int nx, int ny, double penalty_density, double report_density);
        [[nodiscard]] DensityEval evaluate(const Level &) const;
        [[nodiscard]] int nx() const { return nx_; }
        [[nodiscard]] int ny() const { return ny_; }
        [[nodiscard]] double binW() const { return bw_; }
        [[nodiscard]] double binH() const { return bh_; }
    private:
        Region r_;
        int nx_, ny_;
        double target_density_, bw_, bh_;
    };
}

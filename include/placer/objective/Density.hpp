#pragma once
#include "placer/multilevel/Level.hpp"
namespace placer
{
    struct DensityEval
    {
        double penalty=0.0;

        double ofr_penalty=0.0;
        double ofr_report=0.0;

        double total_overflow = 0.0;
        double max_bin_overflow = 0.0;
        double max_density = 0.0;

        double total_movable_area = 0.0;
        double total_fixed_area = 0.0;
        double total_object_area = 0.0;

        int overflow_bins_penalty = 0;
        int overflow_bins_report = 0;


        int overflow_bins_penalty=0,;
        int overflow_bins_report=0;
        
        std::vector<double> gx, gy;
    };
    class DensityGrid
    {
    public:
        DensityGrid(Region r, int nx, int ny, double pd, double rd);
        [[nodiscard]] DensityEval evaluate(const Level &) const;
        [[nodiscard]] int nx() const { return nx_; }
        [[nodiscard]] int ny() const { return ny_; }
        [[nodiscard]] double binW() const { return bw_; }
        [[nodiscard]] double binH() const { return bh_; }

    private:
        Region r_;
        int nx_, ny_;
        double pd_, rd_, bw_, bh_;
    };
}

#pragma once
#include "placer/multilevel/Level.hpp"
namespace placer
{
    struct DensityEval
    {
        double penalty{}, ofr_penalty{}, ofr_report{}, max_density{};
        int overflow_bins_penalty{}, overflow_bins_report{};
        std::vector<double> gx, gy;
    };
    class DensityGrid
    {
    public:
        DensityGrid(Region r, int nx, int ny, double pd, double rd);
        [[nodiscard]] DensityEval evaluate(const Level &) const;
        [[nodiscard]] int nx() const { return nx_; }
        [[nodiscard]] int ny() const { return ny_; }

    private:
        Region r_;
        int nx_, ny_;
        double pd_, rd_, bw_, bh_;
    };
}

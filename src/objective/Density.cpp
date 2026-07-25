#include "placer/objective/Density.hpp"
#include <algorithm>
#include <cmath>
namespace placer
{
  DensityGrid::DensityGrid(Region r, int nx, int ny, double pd, double rd) : r_(r), nx_(std::max(1, nx)), ny_(std::max(1, ny)), pd_(pd), rd_(rd)
  {
    bw_ = (r_.xh - r_.xl) / nx_;
    bh_ = (r_.yh - r_.yl) / ny_;
  }

  static double edgeDeriv(double v, double b)
  {
    if (v < b - 1e-10)
      return 1.0;
    if (v > b + 1e-10)
      return 0.0;
    return 0.5;
  }

  static std::pair<double, double> overlapDeriv(double x, double w, double b0, double b1)
  {
    double right = std::min(x + w, b1), left = std::max(x, b0), ov = right - left;
    if (ov <= 0)
        return {0, 0};

    double dr = edgeDeriv(x + w, b1);
    double dl = (x > b0 + 1e-10) ? 1.0 : ((x < b0 - 1e-10) ? 0.0 : 0.5);

    return {ov, dr - dl};
  }

  static void bounds(const Region &r, double bw, double bh, int nx, int ny, double x, double y, double w, double h, int &ix0, int &ix1, int &iy0, int &iy1)
  {
    ix0 = std::max(0, (int)std::floor((x - r.xl) / bw));
    ix1 = std::min(nx - 1, (int)std::floor((x + w - r.xl - EPS) / bw));
    iy0 = std::max(0, (int)std::floor((y - r.yl) / bh));
    iy1 = std::min(ny - 1, (int)std::floor((y + h - r.yl - EPS) / bh));
  }

  DensityEval DensityGrid::evaluate(
    const Level& level
) const
{
    const int num_bins = nx_ * ny_;
    const double bin_area = bw_ * bh_;

    std::vector<double> movable_area(
        static_cast<std::size_t>(num_bins),
        0.0
    );

    std::vector<double> fixed_area(
        static_cast<std::size_t>(num_bins),
        0.0
    );

    struct CellBinDerivative
    {
        int bin_id;
        double dx;
        double dy;
    };

    std::vector<std::vector<CellBinDerivative>>
        derivatives(level.objects.size());

    DensityEval result;
    result.gx.assign(level.objects.size(), 0.0);
    result.gy.assign(level.objects.size(), 0.0);

    for (std::size_t object_id = 0;
         object_id < level.objects.size();
         ++object_id) {
        const auto& object =
            level.objects[object_id];

        if (object.fixed) {
            result.total_fixed_area += object.area();
        } else {
            result.total_movable_area += object.area();
        }

        // 保留当前 exact overlap 遍历代码。
        // fixed overlap 累加到 fixed_area。
        // movable overlap 累加到 movable_area，
        // 同时保存 dx、dy。
    }

    result.total_object_area =
        result.total_movable_area +
        result.total_fixed_area;

    std::vector<double> gradient_coefficient(
        static_cast<std::size_t>(num_bins),
        0.0
    );

    for (int bin_id = 0;
         bin_id < num_bins;
         ++bin_id) {
        const double density_area =
            movable_area[bin_id] +
            fixed_area[bin_id];

        const double capacity =
            target_density_ * bin_area;

        const double overflow =
            std::max(
                0.0,
                density_area - capacity
            );

        result.total_overflow += overflow;

        result.max_bin_overflow =
            std::max(
                result.max_bin_overflow,
                overflow
            );

        result.penalty +=
            overflow * overflow;

        if (overflow > EPS) {
            ++result.overflow_bins_penalty;
            ++result.overflow_bins_report;
        }

        result.max_density =
            std::max(
                result.max_density,
                density_area /
                    std::max(bin_area, EPS)
            );

        gradient_coefficient[bin_id] =
            2.0 * overflow;
    }

    for (const std::size_t object_id :
         level.movableIds()) {
        for (const auto& record :
             derivatives[object_id]) {
            const double coefficient =
                gradient_coefficient[
                    record.bin_id
                ];

            result.gx[object_id] +=
                coefficient * record.dx;

            result.gy[object_id] +=
                coefficient * record.dy;
        }
    }

    result.ofr_penalty =
        result.total_overflow /
        std::max(
            result.total_object_area,
            EPS
        );

    result.ofr_report =
        result.ofr_penalty;

    return result;
}
}

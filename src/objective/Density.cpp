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
    static double ov(double a, double b, double c, double d) { return std::max(0.0, std::min(b, d) - std::max(a, c)); }
    DensityEval DensityGrid::evaluate(const Level &l) const
    {
        int nbin = nx_ * ny_;
        std::vector<double> mov(nbin, 0), fix(nbin, 0);
        double ma = 0;
        for (auto &o : l.objects)
        {
            double x1 = o.x, x2 = o.x + o.width, y1 = o.y, y2 = o.y + o.height;
            int ix0 = std::max(0, (int)std::floor((x1 - r_.xl) / bw_ - EPS));
            int ix1 = std::min(nx_ - 1, (int)std::floor((x2 - r_.xl) / bw_ + EPS));
            int iy0 = std::max(0, (int)std::floor((y1 - r_.yl) / bh_ - EPS));
            int iy1 = std::min(ny_ - 1, (int)std::floor((y2 - r_.yl) / bh_ + EPS));
            for (int iy = iy0; iy <= iy1; ++iy)
                for (int ix = ix0; ix <= ix1; ++ix)
                {
                    double bx1 = r_.xl + ix * bw_, bx2 = bx1 + bw_, by1 = r_.yl + iy * bh_, by2 = by1 + bh_;
                    double a = ov(x1, x2, bx1, bx2) * ov(y1, y2, by1, by2);
                    if (o.fixed)
                        fix[iy * nx_ + ix] += a;
                    else
                    {
                        mov[iy * nx_ + ix] += a;
                        ma += a;
                    }
                }
        }
        DensityEval e;
        e.gx.assign(l.objects.size(), 0);
        e.gy.assign(l.objects.size(), 0);
        double den = 0, overp = 0, overr = 0, binarea = bw_ * bh_;
        for (int i = 0; i < nbin; ++i)
        {
            double cp = std::max(EPS, pd_ * binarea - fix[i]), cr = std::max(EPS, rd_ * binarea - fix[i]);
            den += cp * cp;
            double rho = mov[i];
            e.max_density = std::max(e.max_density, (mov[i] + fix[i]) / binarea);
            if (rho > cp)
            {
                double z = rho - cp;
                e.penalty += z * z;
                overp += z;
                ++e.overflow_bins_penalty;
            }
            if (rho > cr)
            {
                overr += rho - cr;
                ++e.overflow_bins_report;
            }
        }
        e.penalty /= std::max(EPS, den);
        e.ofr_penalty = overp / std::max(EPS, ma);
        e.ofr_report = overr / std::max(EPS, ma);
        return e;
    }
}

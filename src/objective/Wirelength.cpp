#include "placer/objective/Wirelength.hpp"
#include <algorithm>
#include <cmath>
#include <set>
namespace placer
{
    static double sx(double v) { return (v > EPS) - (v < -EPS); }
    double netHpwl(const Level &l, const LNet &n)
    {
        if (n.pins.size() < 2)
            return 0;
        double xmin = 0, xmax = 0, ymin = 0, ymax = 0;
        bool f = true;
        for (auto &p : n.pins)
        {
            auto &o = l.objects[p.object_id];
            double x = o.cx() + p.offset_x, y = o.cy() + p.offset_y;
            if (f)
            {
                xmin = xmax = x;
                ymin = ymax = y;
                f = false;
            }
            else
            {
                xmin = std::min(xmin, x);
                xmax = std::max(xmax, x);
                ymin = std::min(ymin, y);
                ymax = std::max(ymax, y);
            }
        }
        return xmax - xmin + ymax - ymin;
    }
    double exactHpwl(const Level &l)
    {
        double h = 0;
        for (auto &n : l.nets)
            h += netHpwl(l, n);
        return h;
    }
    WireEval wirelengthSubgradient(const Level &l, WirelengthMode mode)
    {
        WireEval e;
        e.gx.assign(l.objects.size(), 0);
        e.gy.assign(l.objects.size(), 0);
        for (auto &n : l.nets)
        {
            e.hpwl += netHpwl(l, n);
            size_t d = n.pins.size();
            if (d < 2)
                continue;
            if ((mode == WirelengthMode::PaperL1 || mode == WirelengthMode::B2B) && d <= 3)
            {
                double w = d == 3 ? 0.5 : 1.0;
                for (size_t i = 0; i < d; ++i)
                    for (size_t j = i + 1; j < d; ++j)
                    {
                        auto &a = n.pins[i];
                        auto &b = n.pins[j];
                        double ax = l.objects[a.object_id].cx() + a.offset_x, ay = l.objects[a.object_id].cy() + a.offset_y;
                        double bx = l.objects[b.object_id].cx() + b.offset_x, by = l.objects[b.object_id].cy() + b.offset_y;
                        if (!l.objects[a.object_id].fixed)
                        {
                            e.gx[a.object_id] += w * sx(ax - bx);
                            e.gy[a.object_id] += w * sx(ay - by);
                        }
                        if (!l.objects[b.object_id].fixed)
                        {
                            e.gx[b.object_id] += w * sx(bx - ax);
                            e.gy[b.object_id] += w * sx(by - ay);
                        }
                    }
            }
            else
            {
                double xmin = 1e300, xmax = -1e300, ymin = 1e300, ymax = -1e300;
                for (auto &p : n.pins)
                {
                    auto &o = l.objects[p.object_id];
                    double x = o.cx() + p.offset_x, y = o.cy() + p.offset_y;
                    xmin = std::min(xmin, x);
                    xmax = std::max(xmax, x);
                    ymin = std::min(ymin, y);
                    ymax = std::max(ymax, y);
                }
                const double xtol = 1.0e-10 * std::max({1.0, std::abs(xmin), std::abs(xmax)});
                const double ytol = 1.0e-10 * std::max({1.0, std::abs(ymin), std::abs(ymax)});
                std::set<std::size_t> xlow, xhigh, ylow, yhigh;
                for (auto &p : n.pins)
                {
                    auto &o = l.objects[p.object_id];
                    double x = o.cx() + p.offset_x, y = o.cy() + p.offset_y;
                    if (std::abs(x - xmin) <= xtol) xlow.insert(p.object_id);
                    if (std::abs(x - xmax) <= xtol) xhigh.insert(p.object_id);
                    if (std::abs(y - ymin) <= ytol) ylow.insert(p.object_id);
                    if (std::abs(y - ymax) <= ytol) yhigh.insert(p.object_id);
                }
                if (xmax > xmin + EPS)
                {
                    for (auto id : xhigh) if (!l.objects[id].fixed) e.gx[id] += 1.0 / static_cast<double>(xhigh.size());
                    for (auto id : xlow) if (!l.objects[id].fixed) e.gx[id] -= 1.0 / static_cast<double>(xlow.size());
                }
                if (ymax > ymin + EPS)
                {
                    for (auto id : yhigh) if (!l.objects[id].fixed) e.gy[id] += 1.0 / static_cast<double>(yhigh.size());
                    for (auto id : ylow) if (!l.objects[id].fixed) e.gy[id] -= 1.0 / static_cast<double>(ylow.size());
                }
            }
        }
        return e;
    }
    InterlevelHpwl interlevelHpwlConsistency(const Level &c, const Level &f)
    {
        InterlevelHpwl r;
        r.coarse_hpwl = exactHpwl(c);
        r.fine_hpwl = exactHpwl(f);
        r.delta = r.fine_hpwl - r.coarse_hpwl;
        r.relative_delta = std::abs(r.delta) / std::max(EPS, std::abs(r.coarse_hpwl));
        r.ratio = r.fine_hpwl / std::max(EPS, r.coarse_hpwl);
        r.coarse_nets = c.nets.size();
        r.fine_nets = f.nets.size();
        r.paired_nets = std::min(c.nets.size(), f.nets.size());
        for (size_t i = 0; i < r.paired_nets; ++i)
        {
            double d = std::abs(netHpwl(c, c.nets[i]) - netHpwl(f, f.nets[i]));
            r.sum_abs_net_delta += d;
            r.max_abs_net_delta = std::max(r.max_abs_net_delta, d);
        }
        return r;
    }
}

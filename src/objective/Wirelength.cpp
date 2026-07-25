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
            if (mode == WirelengthMode::PaperL1 || mode == WirelengthMode::B2B)
            {
                std::vector<double> xs(d), ys(d);
                for(size_t i=0;i<d;++i){const auto&p=n.pins[i];const auto&o=l.objects[p.object_id];xs[i]=o.cx()+p.offset_x;ys[i]=o.cy()+p.offset_y;}
                const auto [xmin,xmax]=std::minmax_element(xs.begin(),xs.end());
                const auto [ymin,ymax]=std::minmax_element(ys.begin(),ys.end());
                for (size_t i = 0; i < d; ++i)
                    for (size_t j = i + 1; j < d; ++j)
                    {
                        auto &a = n.pins[i];
                        auto &b = n.pins[j];
                        const double ax=xs[i],ay=ys[i],bx=xs[j],by=ys[j];
                        const double base=d==2?1.0:1.0/static_cast<double>(d-1);
                        const bool xi=d>3&&ax>*xmin&&ax<*xmax, xj=d>3&&bx>*xmin&&bx<*xmax;
                        const bool yi=d>3&&ay>*ymin&&ay<*ymax, yj=d>3&&by>*ymin&&by<*ymax;
                        const double wx=(xi&&xj)?0.0:base, wy=(yi&&yj)?0.0:base;
                        if (!l.objects[a.object_id].fixed)
                        {
                            e.gx[a.object_id] += wx * sx(ax - bx);
                            e.gy[a.object_id] += wy * sx(ay - by);
                        }
                        if (!l.objects[b.object_id].fixed)
                        {
                            e.gx[b.object_id] += wx * sx(bx - ax);
                            e.gy[b.object_id] += wy * sx(by - ay);
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

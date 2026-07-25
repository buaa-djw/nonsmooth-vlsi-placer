#include "placer/optimizer/QuadraticInitializer.hpp"
#include "placer/multilevel/Projector.hpp"
#include <chrono>
#include <cmath>
#include <iostream>
#include <stdexcept>
namespace placer
{
    struct Incident
    {
        size_t net_id;
        double offset_x, offset_y, weight;
    };
    QuadraticResult quadraticInitialize(Level &l, const Region &r, const QuadraticConfig &cfg)
    {
        if (needsNullspaceSeed(l))
            seedGrid(l, r, cfg.seed);
        auto ids = l.movableIds();
        QuadraticResult res;
        if (ids.empty() || cfg.iterations <= 0)
            return res;
        const double ccx = (r.xl + r.xh) * 0.5, ccy = (r.yl + r.yh) * 0.5;
        std::vector<std::vector<Incident>> incident(l.objects.size());
        size_t total_pins = 0, total_inc = 0;
        for (size_t nid = 0; nid < l.nets.size(); ++nid)
        {
            const auto &net = l.nets[nid];
            double w = 1.0 / std::max<size_t>(1, net.pins.size() - 1);
            for (const auto &p : net.pins)
            {
                if (p.object_id >= l.objects.size())
                    throw std::runtime_error("quadratic pin object out of range");
                incident[p.object_id].push_back({nid, p.offset_x, p.offset_y, w});
                ++total_pins;
                ++total_inc;
            }
        }
        if (total_inc != total_pins)
            throw std::runtime_error("quadratic incidence invariant failed");
        std::vector<double> net_cx(l.nets.size(), ccx), net_cy(l.nets.size(), ccy);
        int report = std::max(1, cfg.iterations / 20);
        auto t0 = std::chrono::steady_clock::now();
        for (int it = 0; it < cfg.iterations; ++it)
        {
            for (size_t nid = 0; nid < l.nets.size(); ++nid)
            {
                const auto &net = l.nets[nid];
                if (net.pins.empty())
                    continue;
                double sx = 0, sy = 0, sw = 0;
                double w = 1.0 / std::max<size_t>(1, net.pins.size() - 1);
                for (const auto &p : net.pins)
                {
                    const auto &o = l.objects[p.object_id];
                    sx += w * (o.cx() + p.offset_x);
                    sy += w * (o.cy() + p.offset_y);
                    sw += w;
                }
                net_cx[nid] = sx / std::max(sw, EPS);
                net_cy[nid] = sy / std::max(sw, EPS);
            }
            double move2 = 0;
            for (auto oid : ids)
            {
                auto &o = l.objects[oid];
                double sx = cfg.anchor * ccx, sy = cfg.anchor * ccy, sw = cfg.anchor;
                for (const auto &in : incident[oid])
                {
                    sx += in.weight * (net_cx[in.net_id] - in.offset_x);
                    sy += in.weight * (net_cy[in.net_id] - in.offset_y);
                    sw += in.weight;
                }
                if (sw <= EPS)
                    continue;
                double tx = sx / sw, ty = sy / sw;
                double oldx = o.cx(), oldy = o.cy();
                double nx = (1 - cfg.damping) * oldx + cfg.damping * tx, ny = (1 - cfg.damping) * oldy + cfg.damping * ty;
                double dx = nx - oldx, dy = ny - oldy;
                o.setCenter(nx, ny);
                projectObject(o, r);
                move2 += dx * dx + dy * dy;
            }
            res.iterations = it + 1;
            res.rms = std::sqrt(move2 / std::max<size_t>(1, 2 * ids.size()));
            if ((it + 1) % report == 0 || it == 0 || res.rms <= cfg.tolerance)
            {
                double el = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
                std::cout << "[quadratic] iteration=" << (it + 1) << " rms=" << res.rms << " elapsed=" << el << std::endl;
            }
            if (res.rms <= cfg.tolerance)
                break;
        }
        return res;
    }
}

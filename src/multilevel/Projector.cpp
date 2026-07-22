#include "placer/multilevel/Projector.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>
#include <stdexcept>
namespace placer
{
    void projectObject(LObject &o, const Region &r)
    {
        if (o.fixed)
            return;
        double left = -0.5 * o.width, right = 0.5 * o.width, bottom = -0.5 * o.height, top = 0.5 * o.height;
        if (o.projection_bbox)
        {
            left = o.projection_bbox->xl;
            right = o.projection_bbox->xh;
            bottom = o.projection_bbox->yl;
            top = o.projection_bbox->yh;
        }
        double lo_cx = r.xl - left, hi_cx = r.xh - right;
        double lo_cy = r.yl - bottom, hi_cy = r.yh - top;
        if (lo_cx > hi_cx + EPS || lo_cy > hi_cy + EPS)
            throw std::runtime_error("object/envelope does not fit in placement region: " + o.name);
        o.setCenter(std::min(std::max(o.cx(), lo_cx), hi_cx), std::min(std::max(o.cy(), lo_cy), hi_cy));
    }
    void projectLevel(Level &l, const Region &r)
    {
        for (auto &o : l.objects)
            projectObject(o, r);
    }
    bool needsNullspaceSeed(const Level &l)
    {
        bool any = false;
        double x = 0, y = 0;
        for (auto &o : l.objects)
            if (!o.fixed)
            {
                if (!any)
                {
                    x = o.x;
                    y = o.y;
                    any = true;
                }
                else if (std::abs(o.x - x) > EPS || std::abs(o.y - y) > EPS)
                    return false;
            }
        return any;
    }
    void seedGrid(Level &l, const Region &r, int seed)
    {
        auto ids = l.movableIds();
        if (ids.empty())
            return;
        size_t n = ids.size();
        int nx = std::max(1, (int)std::ceil(std::sqrt((double)n * (r.xh - r.xl) / std::max(EPS, r.yh - r.yl))));
        int ny = std::max(1, (int)std::ceil((double)n / (double)nx));
        std::vector<std::pair<double, double>> pts;
        for (int j = 0; j < ny; ++j)
            for (int i = 0; i < nx; ++i)
            {
                double x = r.xl + (i + 0.5) * (r.xh - r.xl) / nx;
                double y = r.yl + (j + 0.5) * (r.yh - r.yl) / ny;
                pts.push_back({x, y});
            }
        std::mt19937 gen((uint32_t)seed);
        std::shuffle(pts.begin(), pts.end(), gen);
        for (size_t k = 0; k < n; ++k)
        {
            auto &o = l.objects[ids[k]];
            o.setCenter(pts[k].first, pts[k].second);
            projectObject(o, r);
        }
    }
}

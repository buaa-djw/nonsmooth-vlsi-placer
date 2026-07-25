#include "placer/multilevel/Projector.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include "placer/util/PythonRandom.hpp"
#include <stdexcept>
namespace placer
{
    ProjectionStats projectObject(LObject &o, const Region &r)
    {
        if (o.fixed)
            return {};
        const double old_x=o.x, old_y=o.y;
        double left = -0.5 * o.width, right = 0.5 * o.width, bottom = -0.5 * o.height, top = 0.5 * o.height;
        if (o.projection_bbox)
        {
            left = std::min(left, o.projection_bbox->xl);
            right = std::max(right, o.projection_bbox->xh);
            bottom = std::min(bottom, o.projection_bbox->yl);
            top = std::max(top, o.projection_bbox->yh);
        }
        double lo_cx = r.xl - left, hi_cx = r.xh - right;
        double lo_cy = r.yl - bottom, hi_cy = r.yh - top;
        if (lo_cx > hi_cx + EPS || lo_cy > hi_cy + EPS)
            throw std::runtime_error("object/envelope does not fit in placement region: " + o.name);
        o.setCenter(std::min(std::max(o.cx(), lo_cx), hi_cx), std::min(std::max(o.cy(), lo_cy), hi_cy));
        const double shift=std::hypot(o.x-old_x,o.y-old_y);
        return {shift>0.0?1:0,shift};
    }
    ProjectionStats projectLevel(Level &l, const Region &r)
    {
        ProjectionStats total;
        for (auto &o : l.objects)
        { auto s=projectObject(o,r); total.projected_objects+=s.projected_objects; total.max_shift=std::max(total.max_shift,s.max_shift); }
        return total;
    }
    bool needsNullspaceSeed(const Level &l)
    {
        const std::vector<std::size_t> ids = l.movableIds();
        if (ids.size() < 2) return false;
        double xmin = l.objects[ids.front()].x;
        double xmax = xmin;
        double ymin = l.objects[ids.front()].y;
        double ymax = ymin;
        for (const std::size_t id : ids)
        {
            const LObject &object = l.objects[id];
            xmin = std::min(xmin, object.x);
            xmax = std::max(xmax, object.x);
            ymin = std::min(ymin, object.y);
            ymax = std::max(ymax, object.y);
        }
        const double span = (xmax - xmin) + (ymax - ymin);
        return span <= 1.0e-9;
    }
    void seedGrid(Level &l, const Region &r, int seed)
    {
        std::vector<std::size_t> order = l.movableIds();
        if (order.empty()) return;
        const std::size_t count = order.size();
        const double width = std::max(r.xh - r.xl, EPS);
        const double height = std::max(r.yh - r.yl, EPS);
        const int nx = std::max(1, static_cast<int>(std::ceil(std::sqrt(static_cast<double>(count) * width / height))));
        const int ny = std::max(1, static_cast<int>(std::ceil(static_cast<double>(count) / static_cast<double>(nx))));
        PythonRandom rng(static_cast<std::uint64_t>(seed));
        rng.shuffle(order);
        for (std::size_t k = 0; k < order.size(); ++k)
        {
            const int ix = static_cast<int>(k % static_cast<std::size_t>(nx));
            const int iy = static_cast<int>(k / static_cast<std::size_t>(nx));
            const double cx = r.xl + (static_cast<double>(ix) + 0.5) * (r.xh - r.xl) / static_cast<double>(nx);
            const double cy = r.yl + (static_cast<double>(iy) + 0.5) * (r.yh - r.yl) / static_cast<double>(ny);
            LObject &object = l.objects[order[k]];
            object.setCenter(cx, cy);
            (void)projectObject(object, r);
        }
    }
}

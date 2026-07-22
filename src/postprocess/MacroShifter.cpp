#include "placer/postprocess/MacroShifter.hpp"
#include "placer/postprocess/SpatialHash.hpp"
#include <algorithm>
#include <cmath>
namespace placer
{
    MacroShiftStats macroShifting(Level &l, const Region &r, int rings, double gap)
    {
        MacroShiftStats st;
        SpatialHash h(10);
        for (size_t i = 0; i < l.objects.size(); ++i)
            if (l.objects[i].fixed)
                h.add({l.objects[i].x, l.objects[i].y, l.objects[i].x + l.objects[i].width, l.objects[i].y + l.objects[i].height, i});
        auto ids = l.macroIds();
        std::sort(ids.begin(), ids.end(), [&](size_t a, size_t b)
                  {auto&A=l.objects[a],&B=l.objects[b]; if(A.area()!=B.area())return A.area()>B.area(); if(std::max(A.width,A.height)!=std::max(B.width,B.height))return std::max(A.width,A.height)>std::max(B.width,B.height); return A.name<B.name; });
        st.macros = ids.size();
        for (auto id : ids)
        {
            auto &o = l.objects[id];
            double ox = o.x, oy = o.y;
            bool placed = false;
            double step = std::max(1.0, std::min(o.width, o.height));
            for (int ring = 0; ring <= rings && !placed; ++ring)
            {
                for (int dx = -ring; dx <= ring && !placed; ++dx)
                {
                    int dy = ring - std::abs(dx);
                    for (int sy : {-1, 1})
                    {
                        double nx = std::min(std::max(ox + dx * step, r.xl), r.xh - o.width);
                        double ny = std::min(std::max(oy + sy * dy * step, r.yl), r.yh - o.height);
                        Rect rr{nx, ny, nx + o.width, ny + o.height, id};
                        if (!h.collides(rr, gap))
                        {
                            o.x = nx;
                            o.y = ny;
                            placed = true;
                            break;
                        }
                        if (dy == 0)
                            break;
                    }
                }
            }
            if (placed)
            {
                double d = std::hypot(o.x - ox, o.y - oy);
                if (d > EPS)
                {
                    ++st.moved;
                    st.total_displacement += d;
                }
                h.add({o.x, o.y, o.x + o.width, o.y + o.height, id});
            }
            else
                ++st.failed;
        }
        return st;
    }
}

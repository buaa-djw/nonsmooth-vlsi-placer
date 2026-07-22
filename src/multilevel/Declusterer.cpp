#include "placer/multilevel/Declusterer.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace placer
{
    DeclusterStats decluster(Level &f, Level &c, const Region &r)
    {
        if (!c.fine_to_coarse || c.fine_to_coarse->size() != f.objects.size())
            throw std::runtime_error("invalid fine_to_coarse for decluster");
        DeclusterStats s;
        s.parents = c.objects.size();
        for (size_t pid = 0; pid < c.objects.size(); ++pid)
        {
            auto &p = c.objects[pid];
            double ax = p.cx(), ay = p.cy(), ox = ax, oy = ay;
            for (size_t cid = 0; cid < f.objects.size(); ++cid)
                if ((*c.fine_to_coarse)[cid] == pid && !f.objects[cid].fixed)
                {
                    auto it = p.child_offsets.find(cid);
                    double dx = it == p.child_offsets.end() ? 0 : it->second.first, dy = it == p.child_offsets.end() ? 0 : it->second.second;
                    auto &ch = f.objects[cid];
                    ax = std::min(std::max(ax, r.xl + ch.width / 2 - dx), r.xh - ch.width / 2 - dx);
                    ay = std::min(std::max(ay, r.yl + ch.height / 2 - dy), r.yh - ch.height / 2 - dy);
                }
            if (std::abs(ax - ox) > EPS || std::abs(ay - oy) > EPS)
            {
                p.setCenter(ax, ay);
                ++s.shifted_parents;
                s.max_parent_shift = std::max(s.max_parent_shift, std::hypot(ax - ox, ay - oy));
            }
            for (size_t cid = 0; cid < f.objects.size(); ++cid)
                if ((*c.fine_to_coarse)[cid] == pid && !f.objects[cid].fixed)
                {
                    auto it = p.child_offsets.find(cid);
                    double dx = it == p.child_offsets.end() ? 0 : it->second.first, dy = it == p.child_offsets.end() ? 0 : it->second.second;
                    f.objects[cid].setCenter(ax + dx, ay + dy);
                    if (f.objects[cid].x < r.xl - EPS || f.objects[cid].x + f.objects[cid].width > r.xh + EPS)
                    {
                        ++s.impossible_groups;
                        throw std::runtime_error("decluster child group cannot fit region");
                    }
                }
        }
        return s;
    }
}

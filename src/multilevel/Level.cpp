#include "placer/multilevel/Level.hpp"
#include <algorithm>
namespace placer
{
    static double med(std::vector<double> v, double d)
    {
        if (v.empty())
            return d;
        sort(v.begin(), v.end());
        return v[v.size() / 2];
    }
    std::vector<size_t> Level::movableIds() const
    {
        std::vector<size_t> r;
        for (size_t i = 0; i < objects.size(); ++i)
            if (!objects[i].fixed)
                r.push_back(i);
        return r;
    }
    std::vector<size_t> Level::standardMovableIds() const
    {
        std::vector<size_t> r;
        for (size_t i = 0; i < objects.size(); ++i)
            if (!objects[i].fixed && !objects[i].is_macro)
                r.push_back(i);
        return r;
    }
    std::vector<size_t> Level::macroIds() const
    {
        std::vector<size_t> r;
        for (size_t i = 0; i < objects.size(); ++i)
            if (!objects[i].fixed && objects[i].is_macro)
                r.push_back(i);
        return r;
    }
    Level buildLevel0(const PlacementDB &db)
    {
        std::vector<double> hs;
        for (auto &r : db.rows)
            if (r.height > 0)
                hs.push_back(r.height);
        double row = med(hs, 1);
        Level l;
        l.index = 0;
        for (size_t i = 0; i < db.cells.size(); ++i)
        {
            auto &c = db.cells[i];
            l.objects.push_back({c.name, c.width, c.height, c.x, c.y, c.fixed, !c.fixed && c.height > 1.5 * row + EPS, {i}, {}, {}, std::nullopt});
        }
        for (size_t net_id=0;net_id<db.nets.size();++net_id)
        {
            const auto &n=db.nets[net_id];
            LNet ln{n.name, {}};
            ln.original_net_id=net_id;
            ln.original_net_name=n.name;
            for (auto pid : n.pin_ids)
            {
                auto &p = db.pins[pid];
                ln.pins.push_back({p.cell_id, p.offset_x, p.offset_y});
            }
            if (ln.pins.size() >= 2)
                l.nets.push_back(ln);
        }
        return l;
    }
}

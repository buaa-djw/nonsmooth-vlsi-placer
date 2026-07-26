#include "placer/database/PlacementDB.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace placer
{
    CellId PlacementDB::addCell(const std::string &n, double w, double h, bool t)
    {
        if (n.empty()) throw std::runtime_error("cell name is empty");
        if (!std::isfinite(w) || !std::isfinite(h) || w <= 0.0 || h <= 0.0)
            throw std::runtime_error("invalid dimensions for cell: " + n);
        if (cell_name_to_id.count(n))
            throw std::runtime_error("duplicate cell: " + n);
        CellId id = cells.size();
        cell_name_to_id[n] = id;
        cells.push_back({n, w, h, 0, 0, t, t, "N"});
        return id;
    }
    NetId PlacementDB::addNet(const std::string &n)
    {
        if (n.empty()) throw std::runtime_error("net name is empty");
        NetId id = nets.size();
        nets.push_back({n, {}});
        return id;
    }
    PinId PlacementDB::addPin(CellId c, NetId n, double ox, double oy, const std::string &d)
    {
        if (c >= cells.size()) throw std::runtime_error("pin cell_id out of range");
        if (n >= nets.size()) throw std::runtime_error("pin net_id out of range");
        if (!std::isfinite(ox) || !std::isfinite(oy)) throw std::runtime_error("pin offset is not finite");
        PinId id = pins.size();
        pins.push_back({c, n, ox, oy, d});
        nets[n].pin_ids.push_back(id);
        return id;
    }
    Region PlacementDB::region() const
    {
        if (!rows.empty())
        {
            double xl = rows[0].x_start, xh = rows[0].x_end, yl = rows[0].y, yh = rows[0].y + rows[0].height;
            for (auto &r : rows)
            {
                xl = std::min(xl, r.x_start);
                xh = std::max(xh, r.x_end);
                yl = std::min(yl, r.y);
                yh = std::max(yh, r.y + r.height);
            }
            if (xh > xl && yh > yl)
                return {xl, xh, yl, yh};
        }
        if (cells.empty())
            return {};
        double xl = cells[0].x, xh = cells[0].x + cells[0].width, yl = cells[0].y, yh = cells[0].y + cells[0].height;
        for (auto &c : cells)
        {
            xl = std::min(xl, c.x);
            xh = std::max(xh, c.x + c.width);
            yl = std::min(yl, c.y);
            yh = std::max(yh, c.y + c.height);
        }
        return {xl, std::max(xh, xl + 1.0), yl, std::max(yh, yl + 1.0)};
    }
    void PlacementDB::validate() const
    {
        if (cell_name_to_id.size() != cells.size()) throw std::runtime_error("cell name map size mismatch");
        for (std::size_t i = 0; i < cells.size(); ++i) {
            const auto &c = cells[i];
            const auto it = cell_name_to_id.find(c.name);
            if (it == cell_name_to_id.end() || it->second != i) throw std::runtime_error("cell name map mismatch: " + c.name);
            if (c.terminal && !c.fixed) throw std::runtime_error("terminal is not fixed: " + c.name);
            if (!std::isfinite(c.width) || !std::isfinite(c.height) || c.width <= 0.0 || c.height <= 0.0) throw std::runtime_error("invalid cell dimensions: " + c.name);
            if (!std::isfinite(c.x) || !std::isfinite(c.y)) throw std::runtime_error("non-finite cell position: " + c.name);
        }
        std::vector<unsigned> ownership(pins.size(), 0U);
        for (std::size_t nid = 0; nid < nets.size(); ++nid) for (const auto pid : nets[nid].pin_ids) {
            if (pid >= pins.size()) throw std::runtime_error("net pin_id out of range: " + nets[nid].name);
            if (pins[pid].net_id != nid) throw std::runtime_error("pin net_id mismatch: " + nets[nid].name);
            ++ownership[pid];
        }
        for (std::size_t pid = 0; pid < pins.size(); ++pid) {
            const auto &p = pins[pid];
            if (p.cell_id >= cells.size() || p.net_id >= nets.size()) throw std::runtime_error("pin endpoint out of range");
            if (ownership[pid] != 1U) throw std::runtime_error("pin does not belong to exactly one net");
            if (!std::isfinite(p.offset_x) || !std::isfinite(p.offset_y)) throw std::runtime_error("non-finite pin offset");
        }
        for (const auto &r : rows) {
            if (!std::isfinite(r.y) || !std::isfinite(r.height) || !std::isfinite(r.x_start) || !std::isfinite(r.x_end) || !std::isfinite(r.site_width) || !std::isfinite(r.site_spacing) || r.height <= 0.0 || r.site_width <= 0.0 || r.site_spacing <= 0.0 || r.num_sites <= 0) throw std::runtime_error("invalid row geometry");
            const double expected = r.x_start + static_cast<double>(r.num_sites) * r.site_spacing;
            if (std::abs(r.x_end - expected) > EPS * std::max({1.0, std::abs(r.x_end), std::abs(expected)})) throw std::runtime_error("row x_end invariant mismatch");
        }
    }
}

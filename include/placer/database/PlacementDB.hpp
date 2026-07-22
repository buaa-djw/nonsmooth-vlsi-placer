#pragma once
#include "placer/Common.hpp"
#include <map>
#include <string>
#include <vector>
namespace placer
{
    struct Cell
    {
        std::string name;
        double width{}, height{}, x{}, y{};
        bool fixed{}, terminal{};
        std::string orientation{"N"};
        [[nodiscard]] double area() const { return width * height; }
        [[nodiscard]] double cx() const { return x + 0.5 * width; }
        [[nodiscard]] double cy() const { return y + 0.5 * height; }
    };
    struct Pin
    {
        CellId cell_id{};
        NetId net_id{};
        double offset_x{}, offset_y{};
        std::string direction;
    };
    struct Net
    {
        std::string name;
        std::vector<PinId> pin_ids;
    };
    struct Row
    {
        double y{}, height{}, x_start{}, x_end{}, site_width{}, site_spacing{};
        int num_sites{};
    };
    struct PlacementDB
    {
        std::vector<Cell> cells;
        std::vector<Pin> pins;
        std::vector<Net> nets;
        std::vector<Row> rows;
        std::map<std::string, CellId> cell_name_to_id;
        CellId addCell(const std::string &, double, double, bool);
        NetId addNet(const std::string &);
        PinId addPin(CellId, NetId, double, double, const std::string &);
        [[nodiscard]] Region region() const;
    };
}

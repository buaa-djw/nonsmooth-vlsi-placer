#pragma once
#include "placer/Common.hpp"
#include <cstddef>
#include <map>
#include <set>
#include <utility>
#include <vector>
namespace placer
{
struct Rect
{
    double xl{};
    double yl{};
    double xh{};
    double yh{};
    std::size_t id{};
};
[[nodiscard]] bool rectsOverlap(const Rect& first, const Rect& second, double gap = 0.0);
class SpatialHash
{
public:
    explicit SpatialHash(double cell_size);
    void add(const Rect& rectangle);
    [[nodiscard]] bool collides(const Rect& rectangle, double gap = 0.0) const;
    [[nodiscard]] std::vector<std::pair<int, int>> keys(const Rect& rectangle) const;
private:
    double cell_size_;
    std::vector<Rect> rectangles_;
    std::map<std::pair<int, int>, std::vector<std::size_t>> buckets_;
};
}

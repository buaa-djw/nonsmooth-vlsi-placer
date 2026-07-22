#include "placer/postprocess/SpatialHash.hpp"
#include <algorithm>
#include <cmath>
namespace placer
{
bool rectsOverlap(const Rect &a, const Rect &b, double g)
{ return a.xl < b.xh + g - EPS && a.xh + g > b.xl + EPS && a.yl < b.yh + g - EPS && a.yh + g > b.yl + EPS; }
SpatialHash::SpatialHash(double c) : cell_size_(std::max(EPS, c)) {}
std::vector<std::pair<int, int>> SpatialHash::keys(const Rect& rectangle) const
{
    const int ix0 = static_cast<int>(std::floor(rectangle.xl / cell_size_));
    const int ix1 = static_cast<int>(std::floor((rectangle.xh - EPS) / cell_size_));
    const int iy0 = static_cast<int>(std::floor(rectangle.yl / cell_size_));
    const int iy1 = static_cast<int>(std::floor((rectangle.yh - EPS) / cell_size_));
    std::vector<std::pair<int, int>> result;
    for (int iy = iy0; iy <= iy1; ++iy) for (int ix = ix0; ix <= ix1; ++ix) result.emplace_back(ix, iy);
    return result;
}
void SpatialHash::add(const Rect& rectangle)
{
    const std::size_t index = rectangles_.size(); rectangles_.push_back(rectangle);
    for (const auto& key : keys(rectangle)) buckets_[key].push_back(index);
}
bool SpatialHash::collides(const Rect& rectangle, double gap) const
{
    std::set<std::size_t> seen;
    for (const auto& key : keys(rectangle)) {
        const auto it = buckets_.find(key); if (it == buckets_.end()) continue;
        for (const std::size_t index : it->second) if (seen.insert(index).second && rectsOverlap(rectangle, rectangles_.at(index), gap)) return true;
    }
    return false;
}
}

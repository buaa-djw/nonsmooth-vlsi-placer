#pragma once
#include "placer/Common.hpp"
#include <map>
#include <vector>
namespace placer { struct Rect{double xl{},yl{},xh{},yh{}; size_t id{};}; [[nodiscard]] bool rectsOverlap(const Rect&, const Rect&, double gap=0.0); class SpatialHash{public: explicit SpatialHash(double cell_size); void add(const Rect&); [[nodiscard]] bool collides(const Rect&, double gap=0.0) const; private: double cs_; std::vector<Rect> rects_;}; }

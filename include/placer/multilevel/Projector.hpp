#pragma once
#include "placer/multilevel/Level.hpp"
namespace placer {
[[nodiscard]] bool needsNullspaceSeed(const Level &);
void seedGrid(Level &, const Region &, int seed);
struct ProjectionStats {
  // Movable objects whose lower-left coordinate actually changed.
  int projected_objects = 0;
  // Euclidean shift of the most displaced object in this pass.
  double max_shift = 0.0;
};
[[nodiscard]] ProjectionStats projectLevel(Level &, const Region &);
// Some legacy callers intentionally ignore per-object statistics; level-wide
// optimization uses projectLevel() and consumes the aggregate result.
ProjectionStats projectObject(LObject &, const Region &);
} // namespace placer

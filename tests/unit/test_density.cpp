#include "../TestSupport.hpp"
#include "placer/io/BookshelfReader.hpp"
#include "placer/objective/Density.hpp"
#include <stdexcept>

static placer::Level oneBin(bool movable) {
  placer::Level l;
  if (movable)
    l.objects.push_back({"m", 3, 10, 0, 0, false, false});
  l.objects.push_back({"f", 5, 10, movable ? 3.0 : 0.0, 0, true, false});
  return l;
}
int main() {
  placer::Region r{0, 10, 0, 10};
  placer::DensityGrid grid(r, 1, 1, 0.6);
  auto mixed = grid.evaluate(oneBin(true));
  CHECK_NEAR(mixed.total_movable_area, 30, 1e-12);
  CHECK_NEAR(mixed.total_fixed_area, 50, 1e-12);
  CHECK_NEAR(mixed.total_overflow, 20, 1e-12);
  CHECK_NEAR(mixed.max_bin_overflow, 20, 1e-12);
  CHECK_NEAR(mixed.penalty, 400, 1e-12);
  CHECK_NEAR(mixed.ofr, 0.25, 1e-12);
  CHECK_NEAR(mixed.gx[1], 0, 1e-12);
  CHECK_NEAR(mixed.gy[1], 0, 1e-12);
  auto fixed = grid.evaluate(oneBin(false));
  CHECK_NEAR(fixed.total_overflow, 0,
             1e-12); // fixed area 50 is below capacity 60
  auto fixed_over = oneBin(false);
  fixed_over.objects[0].width = 8;
  fixed = grid.evaluate(fixed_over);
  CHECK_NEAR(fixed.total_overflow, 20, 1e-12);
  CHECK_NEAR(fixed.penalty, 400, 1e-12);
  CHECK_NEAR(fixed.ofr, 0.25, 1e-12);
  CHECK_NEAR(fixed.gx[0], 0, 1e-12);
  CHECK_NEAR(fixed.gy[0], 0, 1e-12);
  bool rejected = false;
  try {
    placer::DensityGrid bad(r, 1, 1, .6, .7);
    (void)bad;
  } catch (const std::runtime_error &) {
    rejected = true;
  }
  CHECK(rejected);
  return 0;
}

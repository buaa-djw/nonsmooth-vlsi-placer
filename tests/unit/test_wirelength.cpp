#include "placer/io/BookshelfReader.hpp"
#include "placer/objective/Wirelength.hpp"
#include "../TestSupport.hpp"
int main(){ auto db=placer::loadBookshelf("tests/data/tiny/tiny_basic/tiny_basic.aux"); auto l=placer::buildLevel0(db); auto w=placer::wirelengthSubgradient(l,placer::WirelengthMode::Extrema); CHECK_EQ(w.gx.size(),l.objects.size()); CHECK_EQ(w.gy.size(),l.objects.size()); CHECK_NEAR(w.hpwl,placer::exactHpwl(l),1e-12); return 0; }

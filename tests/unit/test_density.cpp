#include "placer/io/BookshelfReader.hpp"
#include "placer/objective/Density.hpp"
#include "../TestSupport.hpp"
int main(){ auto db=placer::loadBookshelf("tests/data/tiny/tiny_basic/tiny_basic.aux"); auto l=placer::buildLevel0(db); placer::DensityGrid dg(db.region(),4,5,1.0,1.0); auto e=dg.evaluate(l); CHECK_EQ(e.gx.size(),l.objects.size()); CHECK_EQ(e.gy.size(),l.objects.size()); CHECK(e.max_density>=0.0); CHECK(e.penalty>=0.0); return 0; }

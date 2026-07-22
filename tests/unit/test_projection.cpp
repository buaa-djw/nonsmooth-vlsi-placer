#include "placer/io/BookshelfReader.hpp"
#include "placer/multilevel/Projector.hpp"
#include "../TestSupport.hpp"
int main(){ auto db=placer::loadBookshelf("tests/data/tiny/tiny_basic/tiny_basic.aux"); auto l=placer::buildLevel0(db); CHECK(!placer::needsNullspaceSeed(l)); for (auto &o : l.objects) if (!o.fixed) { o.x=0.0; o.y=0.0; } CHECK(placer::needsNullspaceSeed(l)); placer::seedGrid(l,db.region(),42); placer::projectLevel(l,db.region()); CHECK(l.objects[0].x>=db.region().xl); return 0; }

#include "placer/io/BookshelfReader.hpp"
#include "placer/multilevel/Level.hpp"
#include "placer/objective/Wirelength.hpp"
#include <cassert>
#include <cmath>
int main(){ auto db=placer::loadBookshelf("tests/data/tiny/tiny_basic/tiny_basic.aux"); assert(db.cells.size()==3); assert(db.nets.size()==2); assert(db.pins.size()==5); auto r=db.region(); assert(std::fabs(r.xh-20.0)<1e-12); auto l=placer::buildLevel0(db); assert(l.objects.size()==3); assert(placer::exactHpwl(l)>=0.0); return 0; }

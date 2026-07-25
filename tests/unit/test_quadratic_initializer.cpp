#include "placer/io/BookshelfReader.hpp"
#include "placer/optimizer/QuadraticInitializer.hpp"
#include "../TestSupport.hpp"
int main(){ auto db=placer::loadBookshelf("tests/data/tiny/tiny_basic/tiny_basic.aux"); auto l=placer::buildLevel0(db); auto r=placer::quadraticInitialize(l,db.region(),{3,0.75,1e-4,0.0,1}); CHECK_EQ(r.iterations,3); CHECK(r.rms>=0.0); return 0; }

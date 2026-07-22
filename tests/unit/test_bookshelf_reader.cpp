#include "placer/io/BookshelfReader.hpp"
#include "../TestSupport.hpp"
int main(){ auto db=placer::loadBookshelf("tests/data/tiny/tiny_basic/tiny_basic.aux"); CHECK_EQ(db.cells.size(),3u); CHECK_EQ(db.nets.size(),2u); CHECK_EQ(db.pins.size(),5u); auto r=db.region(); CHECK_NEAR(r.xl,0.0,1e-12); CHECK_NEAR(r.xh,20.0,1e-12); CHECK_EQ(db.rows.size(),2u); return 0; }

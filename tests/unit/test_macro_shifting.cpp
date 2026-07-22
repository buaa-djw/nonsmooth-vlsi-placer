#include "placer/io/BookshelfReader.hpp"
#include "placer/postprocess/MacroShifter.hpp"
#include "../TestSupport.hpp"
int main(){ auto db=placer::loadBookshelf("tests/data/tiny/tiny_basic/tiny_basic.aux"); auto l=placer::buildLevel0(db); auto st=placer::macroShifting(l,db.region(),1.0,1,0.0); CHECK_EQ(st.failed,0u); CHECK(st.total_displacement>=0.0); return 0; }

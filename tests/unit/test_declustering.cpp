#include "placer/io/BookshelfReader.hpp"
#include "placer/multilevel/Clusterer.hpp"
#include "placer/multilevel/Declusterer.hpp"
#include "../TestSupport.hpp"
int main(){ auto db=placer::loadBookshelf("tests/data/tiny/tiny_basic/tiny_basic.aux"); auto fine=placer::buildLevel0(db); auto coarse=placer::clusterOneLevel(fine,2,256); auto st=placer::decluster(fine,coarse,db.region()); CHECK(st.parents>=1u); CHECK(st.max_parent_shift>=0.0); return 0; }

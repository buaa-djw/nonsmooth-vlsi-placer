#include "placer/io/BookshelfReader.hpp"
#include "placer/multilevel/Clusterer.hpp"
#include "../TestSupport.hpp"
int main(){ auto db=placer::loadBookshelf("tests/data/tiny/tiny_basic/tiny_basic.aux"); auto l0=placer::buildLevel0(db); auto levels=placer::buildHierarchy(l0,{2,2.0,4,256}); CHECK(!levels.empty()); CHECK_EQ(levels.front().index,0); auto coarse=placer::clusterOneLevel(l0,2,256); CHECK(coarse.objects.size()>=2); CHECK_EQ(coarse.index,1); return 0; }

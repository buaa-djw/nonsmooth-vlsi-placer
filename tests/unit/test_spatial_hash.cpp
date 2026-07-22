#include "placer/postprocess/SpatialHash.hpp"
#include "../TestSupport.hpp"
int main(){ placer::SpatialHash h(10.0); placer::Rect r{0,0,10,10,1}; auto keys=h.keys(r); CHECK_EQ(keys.size(),1u); h.add(r); CHECK(h.collides({9,9,11,11,1},0.0)); CHECK(h.collides({9.5,5,10.5,6,2},0.0)); return 0; }

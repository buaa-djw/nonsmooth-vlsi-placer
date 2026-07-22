#include "placer/postprocess/SpatialHash.hpp"
#include <algorithm>
namespace placer { bool rectsOverlap(const Rect&a,const Rect&b,double g){return a.xl < b.xh+g-EPS && a.xh+g > b.xl+EPS && a.yl < b.yh+g-EPS && a.yh+g > b.yl+EPS;} SpatialHash::SpatialHash(double c):cs_(std::max(1.0,c)){} void SpatialHash::add(const Rect&r){rects_.push_back(r);} bool SpatialHash::collides(const Rect&r,double g)const{for(auto&x:rects_)if(x.id!=r.id&&rectsOverlap(x,r,g))return true;return false;} }

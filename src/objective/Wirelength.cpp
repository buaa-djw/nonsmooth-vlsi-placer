#include "placer/objective/Wirelength.hpp"
#include <algorithm>
namespace placer { double exactHpwl(const Level&l){double hp=0; for(auto&n:l.nets){ if(n.pins.size()<2)continue; double xmin=0,xmax=0,ymin=0,ymax=0; bool first=true; for(auto&p:n.pins){auto&o=l.objects[p.object_id]; double x=o.cx()+p.offset_x,y=o.cy()+p.offset_y; if(first){xmin=xmax=x;ymin=ymax=y;first=false;}else{xmin=std::min(xmin,x);xmax=std::max(xmax,x);ymin=std::min(ymin,y);ymax=std::max(ymax,y);}} hp += (xmax-xmin)+(ymax-ymin);} return hp;} }

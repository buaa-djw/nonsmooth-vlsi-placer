#include "placer/multilevel/Declusterer.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace placer
{
    DeclusterStats decluster(Level &f, Level &c, const Region &r)
    {
        if (!c.fine_to_coarse || c.fine_to_coarse->size() != f.objects.size()) throw std::runtime_error("invalid fine_to_coarse for decluster");
        std::vector<std::vector<size_t>> children(c.objects.size());
        for(size_t cid=0; cid<f.objects.size(); ++cid){ size_t pid=(*c.fine_to_coarse)[cid]; if(pid>=c.objects.size()) throw std::runtime_error("invalid fine_to_coarse for decluster"); children[pid].push_back(cid); }
        DeclusterStats s; s.parents=c.objects.size();
        for(size_t pid=0; pid<c.objects.size(); ++pid){ auto&p=c.objects[pid]; double ax=p.cx(), ay=p.cy(), ox=ax, oy=ay; double lo=ax, hi=ax, loy=ay, hiy=ay; bool any=false;
            for(auto cid:children[pid]) if(!f.objects[cid].fixed){ auto it=p.child_offsets.find(cid); double dx=it==p.child_offsets.end()?0:it->second.first, dy=it==p.child_offsets.end()?0:it->second.second; auto&ch=f.objects[cid]; lo=any?std::max(lo,r.xl+0.5*ch.width-dx):r.xl+0.5*ch.width-dx; hi=any?std::min(hi,r.xh-0.5*ch.width-dx):r.xh-0.5*ch.width-dx; loy=any?std::max(loy,r.yl+0.5*ch.height-dy):r.yl+0.5*ch.height-dy; hiy=any?std::min(hiy,r.yh-0.5*ch.height-dy):r.yh-0.5*ch.height-dy; any=true; }
            if(any){ if(lo>hi+EPS || loy>hiy+EPS){ ++s.impossible_groups; throw std::runtime_error("decluster child group cannot fit region"); } ax=std::min(std::max(ax,lo),hi); ay=std::min(std::max(ay,loy),hiy); }
            if(std::abs(ax-ox)>EPS || std::abs(ay-oy)>EPS){ p.setCenter(ax,ay); ++s.shifted_parents; s.max_parent_shift=std::max(s.max_parent_shift,std::hypot(ax-ox,ay-oy)); }
            for(auto cid:children[pid]) if(!f.objects[cid].fixed){ auto it=p.child_offsets.find(cid); double dx=it==p.child_offsets.end()?0:it->second.first, dy=it==p.child_offsets.end()?0:it->second.second; f.objects[cid].setCenter(ax+dx,ay+dy); }
        }
        return s;
    }
}

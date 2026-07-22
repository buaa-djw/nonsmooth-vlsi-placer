#include "placer/postprocess/MacroShifter.hpp"
#include "placer/postprocess/SpatialHash.hpp"
#include <algorithm>
#include <cmath>
#include <optional>
namespace placer
{
namespace {
double medianOr(std::vector<double> values, double fallback){ if(values.empty()) return fallback; std::sort(values.begin(), values.end()); const auto n=values.size(); return n%2?values[n/2]:0.5*(values[n/2-1]+values[n/2]); }
std::vector<std::pair<int,int>> ringCandidates(int ring){ std::vector<std::pair<int,int>> c; if(ring==0){c.emplace_back(0,0); return c;} for(int dx=-ring; dx<=ring; ++dx){ int dy=ring-std::abs(dx); c.emplace_back(dx,dy); if(dy!=0) c.emplace_back(dx,-dy);} return c; }
struct Candidate{ double score{}, x{}, y{}; };
}
MacroShiftStats macroShifting(Level &l, const Region &r, double row_height, int rings, double gap)
{
    MacroShiftStats st;
    std::vector<double> macro_sizes; for (const auto &o:l.objects) if(!o.fixed && o.is_macro) macro_sizes.push_back(std::max(o.width,o.height));
    const double cell_size=std::max(row_height, medianOr(macro_sizes,row_height)); SpatialHash h(cell_size);
    for (size_t i=0;i<l.objects.size();++i) if(l.objects[i].fixed) h.add({l.objects[i].x,l.objects[i].y,l.objects[i].x+l.objects[i].width,l.objects[i].y+l.objects[i].height,i});
    auto ids=l.macroIds(); std::sort(ids.begin(), ids.end(), [&](size_t a,size_t b){auto&A=l.objects[a],&B=l.objects[b]; if(A.area()!=B.area())return A.area()>B.area(); if(std::max(A.width,A.height)!=std::max(B.width,B.height))return std::max(A.width,A.height)>std::max(B.width,B.height); return A.name<B.name;});
    st.macros=ids.size(); const double step=std::max(row_height, std::min((r.xh-r.xl)/100.0,(r.yh-r.yl)/100.0));
    for(auto id:ids){ auto&o=l.objects[id]; const double ox=o.x, oy=o.y; std::optional<Candidate> best;
        for(int ring=0; ring<=rings; ++ring){ best.reset(); for(auto [dx,dy]:ringCandidates(ring)){ const double x=std::min(std::max(ox+static_cast<double>(dx)*step,r.xl), std::max(r.xl,r.xh-o.width)); const double y=std::min(std::max(oy+static_cast<double>(dy)*step,r.yl), std::max(r.yl,r.yh-o.height)); Rect rr{x,y,x+o.width,y+o.height,id}; if(h.collides(rr,gap)) continue; const double score=(x-ox)*(x-ox)+(y-oy)*(y-oy); if(!best || score<best->score) best=Candidate{score,x,y}; } if(best) break; }
        if(!best){ ++st.failed; h.add({o.x,o.y,o.x+o.width,o.y+o.height,id}); continue; }
        o.x=best->x; o.y=best->y; double d=std::hypot(o.x-ox,o.y-oy); if(d>EPS){++st.moved; st.total_displacement+=d;} h.add({o.x,o.y,o.x+o.width,o.y+o.height,id}); }
    return st;
}
}

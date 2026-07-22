#include "placer/postprocess/WhitespaceAllocator.hpp"
#include "placer/multilevel/Projector.hpp"
#include <algorithm>
#include <cmath>
#include <memory>
#include <set>
#include <stdexcept>
namespace placer
{
    namespace
    {
        struct SliceNode
        {
            std::vector<size_t> ids;
            Region bounds;
            double demand{};
            char axis{};
            std::unique_ptr<SliceNode> left;
            std::unique_ptr<SliceNode> right;
        };
        double rectOverlapArea(double ax0,double ax1,double ay0,double ay1,double bx0,double bx1,double by0,double by1)
        {
            return std::max(0.0, std::min(ax1,bx1)-std::max(ax0,bx0))*std::max(0.0, std::min(ay1,by1)-std::max(ay0,by0));
        }
        std::unique_ptr<SliceNode> buildSliceTree(const Level &level, std::vector<size_t> ids, const Region &bounds, int leaf_size)
        {
            auto node=std::make_unique<SliceNode>(); node->ids=ids; node->bounds=bounds;
            if((int)ids.size()<=std::max(1,leaf_size)) return node;
            double sx=bounds.xh-bounds.xl, sy=bounds.yh-bounds.yl; bool split_x=sx>=sy;
            std::sort(ids.begin(), ids.end(), [&](size_t a,size_t b){ const auto &oa=level.objects[a], &ob=level.objects[b]; double ka=split_x?oa.cx():oa.cy(), kb=split_x?ob.cx():ob.cy(); if(std::abs(ka-kb)>EPS) return ka<kb; return oa.name<ob.name; });
            size_t mid=ids.size()/2; if(mid==0 || mid>=ids.size()) return node;
            std::vector<size_t> l(ids.begin(),ids.begin()+(long)mid), r(ids.begin()+(long)mid,ids.end());
            node->axis=split_x?'x':'y';
            if(split_x){ double cut=0.5*(level.objects[ids[mid-1]].cx()+level.objects[ids[mid]].cx()); cut=std::min(std::max(cut,bounds.xl),bounds.xh); node->left=buildSliceTree(level,l,{bounds.xl,cut,bounds.yl,bounds.yh},leaf_size); node->right=buildSliceTree(level,r,{cut,bounds.xh,bounds.yl,bounds.yh},leaf_size); }
            else { double cut=0.5*(level.objects[ids[mid-1]].cy()+level.objects[ids[mid]].cy()); cut=std::min(std::max(cut,bounds.yl),bounds.yh); node->left=buildSliceTree(level,l,{bounds.xl,bounds.xh,bounds.yl,cut},leaf_size); node->right=buildSliceTree(level,r,{bounds.xl,bounds.xh,cut,bounds.yh},leaf_size); }
            return node;
        }
        double computeDemand(SliceNode &node, const Level &level, const std::vector<Region> &fixed_rects, double target_density)
        {
            double movable_area=0; for(auto id:node.ids) movable_area+=level.objects[id].area();
            double fixed_area=0; for(const auto &r:fixed_rects) fixed_area+=rectOverlapArea(node.bounds.xl,node.bounds.xh,node.bounds.yl,node.bounds.yh,r.xl,r.xh,r.yl,r.yh);
            node.demand = movable_area/std::max(target_density,EPS)+fixed_area;
            if(node.left && node.right) node.demand=std::max(node.demand, computeDemand(*node.left,level,fixed_rects,target_density)+computeDemand(*node.right,level,fixed_rects,target_density));
            return node.demand;
        }
        double affineMap(double value,double old0,double old1,double new0,double new1){ if(old1<=old0+EPS) return 0.5*(new0+new1); double t=std::min(1.0,std::max(0.0,(value-old0)/(old1-old0))); return new0+t*(new1-new0); }
        void allocateSlice(const SliceNode &node, Level &level, const Region &nb, double min_fraction)
        {
            if(!node.left || !node.right || node.axis==0){ for(auto id:node.ids){ auto &o=level.objects[id]; o.setCenter(affineMap(o.cx(),node.bounds.xl,node.bounds.xh,nb.xl,nb.xh), affineMap(o.cy(),node.bounds.yl,node.bounds.yh,nb.yl,nb.yh)); } return; }
            double total=std::max(node.left->demand+node.right->demand,EPS); double frac=std::min(1.0-min_fraction,std::max(min_fraction,node.left->demand/total));
            if(node.axis=='x'){ double cut=nb.xl+frac*(nb.xh-nb.xl); allocateSlice(*node.left,level,{nb.xl,cut,nb.yl,nb.yh},min_fraction); allocateSlice(*node.right,level,{cut,nb.xh,nb.yl,nb.yh},min_fraction); }
            else { double cut=nb.yl+frac*(nb.yh-nb.yl); allocateSlice(*node.left,level,{nb.xl,nb.xh,nb.yl,cut},min_fraction); allocateSlice(*node.right,level,{nb.xl,nb.xh,cut,nb.yh},min_fraction); }
        }
        size_t countLeaves(const SliceNode &n){ return (!n.left)?1:countLeaves(*n.left)+countLeaves(*n.right); }
    }
    WhitespaceAllocationResult allocateWhitespace(Level &level, const Region &region, double target_density, int leaf_size, double min_fraction)
    {
        if(!(min_fraction>0.0 && min_fraction<0.5)) throw std::runtime_error("--wsa-min-fraction must be in (0,0.5)");
        auto macros=level.macroIds(); std::set<size_t> locked(macros.begin(),macros.end()); std::vector<size_t> ids; for(auto id:level.movableIds()) if(!locked.count(id)) ids.push_back(id);
        if(ids.size()<2) return {ids.size(),0,0.0}; std::vector<std::pair<double,double>> before; for(auto id:ids) before.push_back({level.objects[id].cx(),level.objects[id].cy()});
        auto root=buildSliceTree(level,ids,region,leaf_size); std::vector<Region> fixed; for(const auto &o:level.objects) if(o.fixed || (!o.fixed && o.is_macro)) fixed.push_back({o.x,o.x+o.width,o.y,o.y+o.height}); computeDemand(*root,level,fixed,target_density); allocateSlice(*root,level,region,min_fraction); projectLevel(level,region);
        double move2=0; for(size_t k=0;k<ids.size();++k){ auto &o=level.objects[ids[k]]; move2+=(o.cx()-before[k].first)*(o.cx()-before[k].first)+(o.cy()-before[k].second)*(o.cy()-before[k].second); }
        return {ids.size(), countLeaves(*root), std::sqrt(move2/std::max<size_t>(1,2*ids.size()))};
    }
}

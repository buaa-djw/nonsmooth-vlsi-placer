#include "placer/objective/Density.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace placer
{
namespace
{
constexpr double BOUNDARY_RELATIVE_TOLERANCE = 1.0e-14;
bool sameBoundary(double a, double b) { return std::abs(a-b) <= BOUNDARY_RELATIVE_TOLERANCE * std::max({1.0,std::abs(a),std::abs(b)}); }
void finite(double value, const char *field) { if (!std::isfinite(value)) throw std::runtime_error(std::string("non-finite density ") + field); }
struct CandidateRange { int first{}, last{}; bool valid{}; };
CandidateRange candidates(double low, double high, double core_low, double core_high, double bin_size, int count)
{
    if (high < core_low || low > core_high) return {};
    int first = static_cast<int>(std::floor((low-core_low)/bin_size));
    const double first_boundary = core_low + static_cast<double>(first)*bin_size;
    if (sameBoundary(low,first_boundary)) --first; // include bin touched by the low edge
    int last = static_cast<int>(std::floor((high-core_low)/bin_size)); // includes bin touched by high edge
    first=std::max(0,std::min(count-1,first)); last=std::max(0,std::min(count-1,last));
    return {first,last,first<=last};
}
}

Overlap1D exactOverlap1D(double object_low,double object_size,double bin_low,double bin_high)
{
    finite(object_low,"object coordinate"); finite(object_size,"object size"); finite(bin_low,"bin coordinate"); finite(bin_high,"bin coordinate");
    if (object_size < 0.0 || !(bin_high > bin_low)) throw std::runtime_error("invalid interval for density overlap");
    const double object_high=object_low+object_size;
    // Paper Eqs. (9)-(10): exact interval intersection, without smoothing.
    const double length=std::max(0.0,std::min(object_high,bin_high)-std::max(object_low,bin_low));
    double derivative=0.0;
    // At external contact the paper leaves a subdifferential choice. Select the
    // deterministic inward one-sided endpoint (+1 entering, -1 leaving).
    if (sameBoundary(object_high,bin_low)) derivative=1.0;
    else if (sameBoundary(object_low,bin_high)) derivative=-1.0;
    else if (length>0.0) {
        if (object_low>=bin_low && object_high<=bin_high) derivative=0.0;
        else if (object_low<=bin_low && object_high>=bin_high) derivative=0.0;
        else if (object_low<bin_low) derivative=1.0;
        else if (object_high>bin_high) derivative=-1.0;
    }
    return {length,derivative};
}

RectangleOverlap exactRectangleOverlap(double x,double y,double width,double height,double bx0,double bx1,double by0,double by1)
{
    const auto ox=exactOverlap1D(x,width,bx0,bx1), oy=exactOverlap1D(y,height,by0,by1);
    return {ox,oy,ox.length*oy.length};
}

DensityGrid::DensityGrid(DensityConfig config):config_(config)
{
    const auto &r=config_.region;
    if (config_.bins_x<=0 || config_.bins_y<=0) throw std::runtime_error("density bin counts must be positive");
    if (!(r.xh>r.xl) || !(r.yh>r.yl)) throw std::runtime_error("density region must have positive area");
    if (!std::isfinite(config_.target_density) || !(config_.target_density>0.0 && config_.target_density<=1.0)) throw std::runtime_error("target density must be in (0,1]");
    bin_width_=(r.xh-r.xl)/static_cast<double>(config_.bins_x); bin_height_=(r.yh-r.yl)/static_cast<double>(config_.bins_y);
}
DensityGrid::DensityGrid(Region region,int nx,int ny,double target):DensityGrid(DensityConfig{region,nx,ny,target}){}
DensityGrid::DensityGrid(Region region,int nx,int ny,double penalty,double report,unsigned):DensityGrid(region,nx,ny,penalty)
{
    if (penalty!=report) throw std::runtime_error("paper density requires penalty/report density to equal target density");
}

DensityEval DensityGrid::evaluate(const Level &level) const
{
    const auto &region=config_.region; const int nbin=config_.bins_x*config_.bins_y; const double bin_area=bin_width_*bin_height_;
    DensityEval result; result.gx.assign(level.objects.size(),0.0); result.gy.assign(level.objects.size(),0.0);
    result.bin_density_area.assign(static_cast<std::size_t>(nbin),0.0); result.bin_capacity.assign(static_cast<std::size_t>(nbin),config_.target_density*bin_area);
    struct Stencil { int bin; double dx,dy; };
    std::vector<std::vector<Stencil>> stencils(level.objects.size());
    for (std::size_t oid=0;oid<level.objects.size();++oid) {
        const auto &object=level.objects[oid]; const double area=object.area(); finite(area,"object area");
        if (area<0.0) throw std::runtime_error("negative object area in density evaluation");
        result.total_object_area+=area; if(object.fixed)result.total_fixed_area+=area;else result.total_movable_area+=area;
        const auto xr=candidates(object.x,object.x+object.width,region.xl,region.xh,bin_width_,config_.bins_x);
        const auto yr=candidates(object.y,object.y+object.height,region.yl,region.yh,bin_height_,config_.bins_y);
        if(!xr.valid||!yr.valid)continue;
        for(int iy=yr.first;iy<=yr.last;++iy)for(int ix=xr.first;ix<=xr.last;++ix){
            ++result.candidate_pair_count; const double bx0=region.xl+static_cast<double>(ix)*bin_width_,by0=region.yl+static_cast<double>(iy)*bin_height_;
            const auto overlap=exactRectangleOverlap(object.x,object.y,object.width,object.height,bx0,bx0+bin_width_,by0,by0+bin_height_);
            const int bin=iy*config_.bins_x+ix; result.bin_density_area[static_cast<std::size_t>(bin)]+=overlap.area;
            if(overlap.area>0.0)++result.positive_overlap_pair_count;else ++result.touching_pair_count;
            if(!object.fixed){const double dx=overlap.x.translation_subgradient*overlap.y.length,dy=overlap.x.length*overlap.y.translation_subgradient;if(dx!=0.0||dy!=0.0)stencils[oid].push_back({bin,dx,dy});}
        }
    }
    result.raw_overflow.resize(static_cast<std::size_t>(nbin)); result.positive_overflow.resize(static_cast<std::size_t>(nbin)); result.squared_overflow.resize(static_cast<std::size_t>(nbin));
    result.maximum_raw_overflow=-std::numeric_limits<double>::infinity();
    for(int bin=0;bin<nbin;++bin){const auto i=static_cast<std::size_t>(bin);const double density=result.bin_density_area[i];const double raw=density-result.bin_capacity[i],positive=std::max(0.0,raw),square=positive*positive;
        // Paper Eq. (11): D_b <= target_density * bin_area.
        // Paper Eq. (12): unnormalized sum of squared positive overflow.
        result.raw_overflow[i]=raw;result.positive_overflow[i]=positive;result.squared_overflow[i]=square;result.total_bin_overlap+=density;result.total_positive_overflow+=positive;result.quadratic_penalty+=square;
        if(positive>0.0)++result.overflow_bin_count;
        result.maximum_raw_overflow=std::max(result.maximum_raw_overflow,raw);result.maximum_positive_overflow=std::max(result.maximum_positive_overflow,positive);result.maximum_bin_density=std::max(result.maximum_bin_density,density/bin_area);
    }
    result.total_clipped_area=result.total_bin_overlap;result.total_overflow=result.total_positive_overflow;result.max_bin_overflow=result.maximum_positive_overflow;
    for(std::size_t oid=0;oid<level.objects.size();++oid)if(!level.objects[oid].fixed)for(const auto&s:stencils[oid]){const double coefficient=2.0*result.positive_overflow[static_cast<std::size_t>(s.bin)];if(coefficient>0.0){result.gx[oid]+=coefficient*s.dx;result.gy[oid]+=coefficient*s.dy;++result.gradient_contribution_count;}}
    // Paper Eq. (18): denominator is complete area of all movable and fixed objects.
    result.paper_ofr=result.total_object_area>0.0?result.total_positive_overflow/result.total_object_area:0.0;
    result.penalty=result.quadratic_penalty;result.ofr_penalty=result.paper_ofr;result.ofr_report=result.paper_ofr;result.max_density=result.maximum_bin_density;result.overflow_bins_penalty=static_cast<int>(result.overflow_bin_count);result.overflow_bins_report=result.overflow_bins_penalty;
    finite(result.quadratic_penalty,"penalty");finite(result.paper_ofr,"OFR");for(double v:result.gx)finite(v,"x gradient");for(double v:result.gy)finite(v,"y gradient");
    return result;
}
}

#include "placer/objective/Density.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace placer
{
    DensityGrid::DensityGrid(Region r,int nx,int ny,double pd,double rd,unsigned seed):r_(r),nx_(std::max(1,nx)),ny_(std::max(1,ny)),pd_(pd),rd_(rd),random_(seed){bw_=(r_.xh-r_.xl)/nx_; bh_=(r_.yh-r_.yl)/ny_;}
    static bool equal(double a,double b){return std::abs(a-b)<=EPS*std::max({1.0,std::abs(a),std::abs(b)});}
    static double angle(std::mt19937_64&r,double lo,double hi){return std::uniform_real_distribution<double>(lo,hi)(r);}
    static std::pair<double,double> overlapDeriv(double x,double w,double b0,double b1,std::mt19937_64&r){
      const double right=std::min(x+w,b1),left=std::max(x,b0),ov=std::max(0.0,right-left);
      if(equal(x,b1))return {ov,std::cos(angle(r,0.5*std::acos(-1.0),std::acos(-1.0)))};
      if(equal(x+w,b0))return {ov,std::cos(angle(r,0.0,0.5*std::acos(-1.0)))};
      double dr=x+w<b1?1.0:0.0,dl=x>b0?1.0:0.0;
      if(equal(x+w,b1))dr=std::cos(angle(r,0.0,0.5*std::acos(-1.0)));
      if(equal(x,b0))dl=-std::cos(angle(r,0.5*std::acos(-1.0),std::acos(-1.0)));
      return {ov,dr-dl};
    }
    static void bounds(const Region&r,double bw,double bh,int nx,int ny,double x,double y,double w,double h,int&ix0,int&ix1,int&iy0,int&iy1){ ix0=std::max(0,(int)std::floor((x-r.xl)/bw)); ix1=std::min(nx-1,(int)std::floor((x+w-r.xl-EPS)/bw)); iy0=std::max(0,(int)std::floor((y-r.yl)/bh)); iy1=std::min(ny-1,(int)std::floor((y+h-r.yl-EPS)/bh)); }
    DensityEval DensityGrid::evaluate(const Level &l) const
    { int nbin=nx_*ny_; std::vector<double> mov(nbin,0), fix(nbin,0); struct St{int b; double dx,dy;}; std::vector<std::vector<St>> st(l.objects.size()); double binarea=bw_*bh_; DensityEval e;
      for(size_t oid=0;oid<l.objects.size();++oid){ const auto&o=l.objects[oid]; int ix0,ix1,iy0,iy1; bounds(r_,bw_,bh_,nx_,ny_,o.x,o.y,o.width,o.height,ix0,ix1,iy0,iy1); for(int iy=iy0;iy<=iy1;++iy){ double by0=r_.yl+iy*bh_, by1=by0+bh_; auto [oy,doy]=overlapDeriv(o.y,o.height,by0,by1,random_); if(oy<=0) continue; for(int ix=ix0;ix<=ix1;++ix){ double bx0=r_.xl+ix*bw_, bx1=bx0+bw_; auto [ox,dox]=overlapDeriv(o.x,o.width,bx0,bx1,random_); if(ox<=0) continue; int b=iy*nx_+ix; if(o.fixed) fix[b]+=ox*oy; else { mov[b]+=ox*oy; st[oid].push_back({b,dox*oy,doy*ox}); } }} }
      for(const auto&o:l.objects){ e.total_object_area+=o.area(); if(o.fixed)e.total_fixed_area+=o.area();else e.total_movable_area+=o.area(); }
      if(!(e.total_object_area>0.0)) throw std::runtime_error("paper OFR requires positive total object area");
      for(int b=0;b<nbin;++b)e.total_clipped_area+=mov[b]+fix[b];
      std::vector<double> coeff(nbin); e.gx.assign(l.objects.size(),0); e.gy.assign(l.objects.size(),0);
      for(int b=0;b<nbin;++b){
        // Equation (18): fixed and movable overlap share the same, unclamped capacity.
        const double overflow=std::max(0.0,mov[b]+fix[b]-pd_*binarea);
        if(overflow>EPS)++e.overflow_bin_count;
        e.quadratic_penalty+=overflow*overflow; e.total_overflow+=overflow;
        e.max_bin_overflow=std::max(e.max_bin_overflow,overflow);
        coeff[b]=2.0*overflow;
        e.maximum_bin_density=std::max(e.maximum_bin_density,(mov[b]+fix[b])/std::max(binarea,EPS));
      }
      for(auto oid:l.movableIds()) for(const auto&s:st[oid]){ e.gx[oid]+=coeff[s.b]*s.dx; e.gy[oid]+=coeff[s.b]*s.dy; }
      e.paper_ofr=e.total_overflow/e.total_object_area;
      e.penalty=e.quadratic_penalty; e.ofr_penalty=e.paper_ofr; e.ofr_report=e.paper_ofr;
      e.max_density=e.maximum_bin_density; e.overflow_bins_penalty=static_cast<int>(e.overflow_bin_count); e.overflow_bins_report=e.overflow_bins_penalty;
      (void)rd_; // Paper mode has one target density for penalty, OFR, and control.
      return e; }
}

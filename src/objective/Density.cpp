#include "placer/objective/Density.hpp"
#include <algorithm>
#include <cmath>
namespace placer
{
    DensityGrid::DensityGrid(Region r,int nx,int ny,double pd,double rd):r_(r),nx_(std::max(1,nx)),ny_(std::max(1,ny)),pd_(pd),rd_(rd){bw_=(r_.xh-r_.xl)/nx_; bh_=(r_.yh-r_.yl)/ny_;}
    static double edgeDeriv(double v,double b){ if(v<b-1e-10) return 1.0; if(v>b+1e-10) return 0.0; return 0.5; }
    static std::pair<double,double> overlapDeriv(double x,double w,double b0,double b1){ double right=std::min(x+w,b1), left=std::max(x,b0), ov=right-left; if(ov<=0) return {0,0}; double dr=edgeDeriv(x+w,b1); double dl=(x>b0+1e-10)?1.0:((x<b0-1e-10)?0.0:0.5); return {ov,dr-dl}; }
    static void bounds(const Region&r,double bw,double bh,int nx,int ny,double x,double y,double w,double h,int&ix0,int&ix1,int&iy0,int&iy1){ ix0=std::max(0,(int)std::floor((x-r.xl)/bw)); ix1=std::min(nx-1,(int)std::floor((x+w-r.xl-EPS)/bw)); iy0=std::max(0,(int)std::floor((y-r.yl)/bh)); iy1=std::min(ny-1,(int)std::floor((y+h-r.yl-EPS)/bh)); }
    DensityEval DensityGrid::evaluate(const Level &l) const
    { int nbin=nx_*ny_; std::vector<double> mov(nbin,0), fix(nbin,0); struct St{int b; double dx,dy;}; std::vector<std::vector<St>> st(l.objects.size()); double mov_area=0, binarea=bw_*bh_;
      for(size_t oid=0;oid<l.objects.size();++oid){ const auto&o=l.objects[oid]; int ix0,ix1,iy0,iy1; bounds(r_,bw_,bh_,nx_,ny_,o.x,o.y,o.width,o.height,ix0,ix1,iy0,iy1); for(int iy=iy0;iy<=iy1;++iy){ double by0=r_.yl+iy*bh_, by1=by0+bh_; auto [oy,doy]=overlapDeriv(o.y,o.height,by0,by1); if(oy<=0) continue; for(int ix=ix0;ix<=ix1;++ix){ double bx0=r_.xl+ix*bw_, bx1=bx0+bw_; auto [ox,dox]=overlapDeriv(o.x,o.width,bx0,bx1); if(ox<=0) continue; int b=iy*nx_+ix; if(o.fixed) fix[b]+=ox*oy; else { mov[b]+=ox*oy; st[oid].push_back({b,dox*oy,doy*ox}); } }} if(!o.fixed) mov_area+=o.area(); }
      std::vector<double> capP(nbin),capR(nbin),coeff(nbin); double raw2=0, rawP=0, rawR=0; DensityEval e; e.gx.assign(l.objects.size(),0); e.gy.assign(l.objects.size(),0);
      for(int b=0;b<nbin;++b){ capP[b]=std::max(EPS,pd_*binarea-fix[b]); capR[b]=std::max(EPS,rd_*binarea-fix[b]); }
      for(int b=0;b<nbin;++b){ double op=std::max(0.0,mov[b]-capP[b]), orp=std::max(0.0,mov[b]-capR[b]); if(op>EPS) ++e.overflow_bins_penalty; if(orp>EPS) ++e.overflow_bins_report; raw2+=op*op; rawP+=op; rawR+=orp; coeff[b]=2.0*op; e.max_density=std::max(e.max_density,(mov[b]+fix[b])/std::max(binarea,EPS)); }
      for(auto oid:l.movableIds()) for(const auto&s:st[oid]){ e.gx[oid]+=coeff[s.b]*s.dx; e.gy[oid]+=coeff[s.b]*s.dy; }
      e.penalty=raw2; e.ofr_penalty=rawP/std::max(mov_area,EPS); e.ofr_report=rawR/std::max(mov_area,EPS); return e; }
}

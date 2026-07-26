// Consolidated from test_density.cpp
#include "placer/io/BookshelfReader.hpp"
#include "placer/objective/Density.hpp"
#include "../TestSupport.hpp"
int consolidated_density_0(){
    auto db=placer::loadBookshelf("tests/data/tiny/tiny_basic/tiny_basic.aux");
    auto l=placer::buildLevel0(db); placer::DensityGrid dg(db.region(),4,5,1.0);
    auto e=dg.evaluate(l); CHECK_EQ(e.gx.size(),l.objects.size()); CHECK(e.penalty>=0.0);

    placer::Level manual;
    manual.objects.push_back({"fixed",8,8,0,0,true,false});
    manual.objects.push_back({"movable",2,2,8,8,false,false});
    placer::DensityGrid one({0,10,0,10},1,1,0.5);
    auto m=one.evaluate(manual);
    CHECK_NEAR(m.total_object_area,68.0,1e-12);
    CHECK_NEAR(m.total_fixed_area,64.0,1e-12);
    CHECK_NEAR(m.total_movable_area,4.0,1e-12);
    CHECK_NEAR(m.total_overflow,18.0,1e-12);
    CHECK_NEAR(m.quadratic_penalty,324.0,1e-12);
    CHECK_NEAR(m.paper_ofr,18.0/68.0,1e-12);
    CHECK_EQ(m.overflow_bin_count,1u);
    CHECK_NEAR(m.max_bin_overflow,18.0,1e-12);
    CHECK_NEAR(m.gx[0],0.0,0.0); CHECK_NEAR(m.gy[0],0.0,0.0);

    placer::Level outside;
    outside.objects.push_back({"partly-outside-fixed",10,10,-5,0,true,false});
    auto o=one.evaluate(outside);
    CHECK_NEAR(o.total_object_area,100.0,1e-12);
    CHECK_NEAR(o.total_clipped_area,50.0,1e-12);
    CHECK_NEAR(o.paper_ofr,0.0,1e-12);
    placer::Level empty; auto z=one.evaluate(empty); CHECK_NEAR(z.paper_ofr,0.0,0.0); CHECK_NEAR(z.penalty,0.0,0.0);
    return 0;
}

// Consolidated from test_density_boundary.cpp
#include "test_density_support.hpp"
#include "../TestSupport.hpp"
int consolidated_density_1(){auto l=densityLevel({{"touch-right",1,2,4,1,false,false},{"overflow",4,5,5,0,true,false}});placer::DensityGrid g({0,10,0,5},2,1,0.5);auto e=g.evaluate(l);CHECK_NEAR(e.bin_density_area[1],20,1e-12);CHECK_NEAR(e.positive_overflow[1],7.5,1e-12);CHECK_NEAR(e.gx[0],30,1e-12);CHECK(e.touching_pair_count>=1);auto under=densityLevel({{"touch",1,2,4,1,false,false}});auto u=g.evaluate(under);CHECK_NEAR(u.gx[0],0,0);auto vertical=densityLevel({{"touch-up",2,1,1,4,false,false},{"overflow",5,4,0,5,true,false}});placer::DensityGrid gy({0,5,0,10},1,2,.5);auto v=gy.evaluate(vertical);CHECK_NEAR(v.gy[0],30,1e-12);auto corner=placer::exactRectangleOverlap(4,4,1,1,5,10,5,10);CHECK_NEAR(corner.area,0,0);return 0;}

// Consolidated from test_density_configuration_consistency.cpp
#include "test_density_support.hpp"
#include "placer/Config.hpp"
#include "../TestSupport.hpp"
int consolidated_density_2(){auto l=densityLevel({{"a",6,6,0,0,false,false},{"f",5,5,5,5,true,false}});placer::DensityConfig c{{0,10,0,10},2,2,.7};placer::DensityGrid optimization(c),final(c),reload(c);auto a=optimization.evaluate(l),b=final.evaluate(l),d=reload.evaluate(l);CHECK_NEAR(a.total_positive_overflow,b.total_positive_overflow,0);CHECK_NEAR(a.quadratic_penalty,d.quadratic_penalty,0);CHECK_NEAR(a.paper_ofr,d.paper_ofr,0);CHECK_NEAR(a.maximum_positive_overflow,d.maximum_positive_overflow,0);bool legacy=false;try{placer::DensityGrid bad({0,10,0,10},1,1,.7,.8);(void)bad;}catch(const std::runtime_error&){legacy=true;}CHECK(legacy);const char*args[]={"p","a.aux","--target-density","0.7","--penalty-density","0.8"};bool cli=false;try{(void)placer::parseConfig(6,const_cast<char**>(args));}catch(const std::runtime_error&){cli=true;}CHECK(cli);return 0;}

// Consolidated from test_density_conservation.cpp
#include "test_density_support.hpp"
#include "../TestSupport.hpp"
int consolidated_density_3(){auto l=densityLevel({{"inside",4,3,1,1,false,false},{"outside",4,4,-2,8,false,false}});placer::DensityGrid g({0,10,0,10},5,4,1.0);auto e=g.evaluate(l);CHECK_NEAR(e.total_object_area,28,1e-12);CHECK_NEAR(e.total_clipped_area,16,1e-12);CHECK_NEAR(e.total_bin_overlap,16,1e-12);CHECK(e.positive_overlap_pair_count>0);return 0;}

// Consolidated from test_density_descent_step.cpp
#include "test_density_support.hpp"
#include "placer/objective/Wirelength.hpp"
#include "../TestSupport.hpp"
int consolidated_density_4(){auto l=densityLevel({{"movable",2,2,4,1,false,false},{"fixed",4,5,0,0,true,false}});placer::DensityGrid g({0,10,0,5},2,1,.5);auto b=g.evaluate(l);double hp=placer::exactHpwl(l),fx=l.objects[1].x,fy=l.objects[1].y;l.objects[0].x-=.001*b.gx[0];l.objects[0].y-=.001*b.gy[0];auto a=g.evaluate(l);CHECK(a.quadratic_penalty<b.quadratic_penalty);CHECK(a.paper_ofr<=b.paper_ofr);CHECK_NEAR(placer::exactHpwl(l),hp,0);CHECK_NEAR(l.objects[1].x,fx,0);CHECK_NEAR(l.objects[1].y,fy,0);CHECK(l.objects[0].x>=0&&l.objects[0].x+l.objects[0].width<=10);return 0;}

// Consolidated from test_density_empty.cpp
#include "test_density_support.hpp"
#include "../TestSupport.hpp"
int consolidated_density_5(){placer::DensityGrid g({0,10,0,10},1,1,1);placer::Level empty;auto e=g.evaluate(empty);CHECK_NEAR(e.total_object_area,0,0);CHECK_NEAR(e.paper_ofr,0,0);CHECK_NEAR(e.quadratic_penalty,0,0);CHECK(std::isfinite(e.maximum_raw_overflow));auto fixed=densityLevel({{"fixed",1,1,0,0,true,false}});auto f=g.evaluate(fixed);CHECK_NEAR(f.paper_ofr,0,0);CHECK_NEAR(f.gx[0],0,0);return 0;}

// Consolidated from test_density_fixed.cpp
#include "test_density_support.hpp"
#include "../TestSupport.hpp"
int consolidated_density_6(){auto l=densityLevel({{"fixed",8,8,0,0,true,false}});placer::DensityGrid g({0,10,0,10},1,1,0.5);auto e=g.evaluate(l);CHECK_NEAR(e.bin_density_area[0],64,1e-12);CHECK_NEAR(e.bin_capacity[0],50,1e-12);CHECK_NEAR(e.positive_overflow[0],14,1e-12);CHECK_NEAR(e.quadratic_penalty,196,1e-12);CHECK_NEAR(e.paper_ofr,14.0/64.0,1e-12);CHECK_NEAR(e.total_fixed_area,64,1e-12);CHECK_NEAR(e.gx[0],0,0);CHECK_NEAR(e.gy[0],0,0);return 0;}

// Consolidated from test_density_ofr_equation18.cpp
#include "test_density_support.hpp"
#include "../TestSupport.hpp"
int consolidated_density_7(){auto l=densityLevel({{"fixed",8,10,0,0,true,false},{"partial",10,10,5,0,false,false}});placer::DensityGrid g({0,10,0,10},1,1,0.5);auto e=g.evaluate(l);CHECK_NEAR(e.total_fixed_area,80,1e-12);CHECK_NEAR(e.total_movable_area,100,1e-12);CHECK_NEAR(e.total_object_area,180,1e-12);CHECK_NEAR(e.total_clipped_area,130,1e-12);CHECK_NEAR(e.total_positive_overflow,80,1e-12);CHECK_NEAR(e.quadratic_penalty,6400,1e-12);CHECK_NEAR(e.paper_ofr,80.0/180.0,1e-12);return 0;}

// Consolidated from test_density_overlap.cpp
#include "placer/objective/Density.hpp"
#include "../TestSupport.hpp"
static int check(double x,double w,double b0,double b1,double length,double derivative){auto o=placer::exactOverlap1D(x,w,b0,b1);CHECK_NEAR(o.length,length,1e-12);CHECK_NEAR(o.translation_subgradient,derivative,1e-12);return 0;}
int consolidated_density_8(){CHECK_EQ(check(2,3,0,10,3,0),0);CHECK_EQ(check(-2,15,0,10,10,0),0);CHECK_EQ(check(-2,5,0,10,3,1),0);CHECK_EQ(check(8,5,0,10,2,-1),0);CHECK_EQ(check(11,2,0,10,0,0),0);CHECK_EQ(check(-2,2,0,10,0,1),0);CHECK_EQ(check(10,2,0,10,0,-1),0);auto c=placer::exactRectangleOverlap(8,8,4,5,0,10,0,10);CHECK_NEAR(c.x.length,2,1e-12);CHECK_NEAR(c.y.length,2,1e-12);CHECK_NEAR(c.area,4,1e-12);auto n=placer::exactRectangleOverlap(1,1,7,3,0,4,0,10);CHECK_NEAR(n.area,9,1e-12);auto lower=placer::exactRectangleOverlap(1,-2,2,5,0,10,0,10);CHECK_NEAR(lower.area,6,1e-12);CHECK_NEAR(lower.y.translation_subgradient,1,0);auto upper=placer::exactRectangleOverlap(1,8,2,5,0,10,0,10);CHECK_NEAR(upper.area,4,1e-12);CHECK_NEAR(upper.y.translation_subgradient,-1,0);return 0;}

// Consolidated from test_density_penalty_equation12.cpp
#include "test_density_support.hpp"
#include "../TestSupport.hpp"
int consolidated_density_9(){auto l=densityLevel({{"fixed",4,10,0,0,true,false},{"mixed",2,10,4,0,false,false},{"under",4,10,10,0,false,false}});placer::DensityGrid g({0,20,0,10},2,1,0.5);auto e=g.evaluate(l);CHECK_NEAR(e.bin_density_area[0],60,1e-12);CHECK_NEAR(e.bin_density_area[1],40,1e-12);CHECK_NEAR(e.bin_capacity[0],50,1e-12);CHECK_NEAR(e.raw_overflow[0],10,1e-12);CHECK_NEAR(e.raw_overflow[1],-10,1e-12);CHECK_NEAR(e.positive_overflow[0],10,1e-12);CHECK_NEAR(e.squared_overflow[0],100,1e-12);CHECK_NEAR(e.quadratic_penalty,100,1e-12);CHECK_EQ(e.overflow_bin_count,1u);return 0;}

// Consolidated from test_density_properties.cpp
#include "test_density_support.hpp"
#include "../TestSupport.hpp"
#include <random>
int consolidated_density_10(){std::mt19937_64 r(303);std::uniform_real_distribution<double>p(-2,10),s(.1,3);for(int trial=0;trial<20;++trial){placer::Level l;for(int i=0;i<12;++i)l.objects.push_back({"o",s(r),s(r),p(r),p(r),i%5==0,false});placer::DensityGrid g({0,10,0,10},3+trial%4,2+trial%3,.8);auto a=g.evaluate(l),b=g.evaluate(l);CHECK_NEAR(a.total_bin_overlap,b.total_bin_overlap,0);CHECK_NEAR(a.quadratic_penalty,b.quadratic_penalty,0);CHECK_NEAR(a.paper_ofr,b.paper_ofr,0);double clipped=0;for(const auto&o:l.objects)clipped+=placer::exactRectangleOverlap(o.x,o.y,o.width,o.height,0,10,0,10).area;CHECK_NEAR(a.total_bin_overlap,clipped,1e-10);CHECK(a.quadratic_penalty>=0&&a.paper_ofr>=0);std::size_t count=0;for(double v:a.positive_overflow){CHECK(v>=0);count+=v>0;}CHECK_EQ(count,a.overflow_bin_count);CHECK(std::isfinite(a.maximum_raw_overflow));}return 0;}

// Consolidated from test_density_subgradient.cpp
#include "test_density_support.hpp"
#include "../TestSupport.hpp"
int consolidated_density_11(){auto l=densityLevel({{"movable",2,2,4,1,false,false},{"fixed",4,5,0,0,true,false}});placer::DensityGrid g({0,10,0,5},2,1,.5);auto e=g.evaluate(l);double h=1e-6;l.objects[0].x+=h;double pp=g.evaluate(l).quadratic_penalty;l.objects[0].x-=2*h;double pm=g.evaluate(l).quadratic_penalty;l.objects[0].x+=h;double numerical=(pp-pm)/(2*h);CHECK_NEAR(e.gx[0],numerical,1e-6);CHECK_NEAR(e.gx[1],0,0);CHECK_NEAR(e.gy[1],0,0);auto touch=placer::exactOverlap1D(-1,1,0,5);CHECK(touch.translation_subgradient>=0&&touch.translation_subgradient<=1);return 0;}

// Consolidated from test_density_target_density.cpp
#include "test_density_support.hpp"
#include "../TestSupport.hpp"
static int check(double t,double overflow,double penalty,double ofr){auto l=densityLevel({{"cell",9,10,0,0,false,false}});placer::DensityGrid g({0,10,0,10},1,1,t);auto e=g.evaluate(l);CHECK_NEAR(e.bin_capacity[0],100*t,1e-12);CHECK_NEAR(e.total_positive_overflow,overflow,1e-12);CHECK_NEAR(e.quadratic_penalty,penalty,1e-12);CHECK_NEAR(e.paper_ofr,ofr,1e-12);return 0;}int consolidated_density_12(){CHECK_EQ(check(1,0,0,0),0);CHECK_EQ(check(.9,0,0,0),0);CHECK_EQ(check(.8,10,100,1.0/9.0),0);CHECK_EQ(check(.5,40,1600,4.0/9.0),0);return 0;}

int main(){
  if (consolidated_density_0()!=0) return 1;
  if (consolidated_density_1()!=0) return 1;
  if (consolidated_density_2()!=0) return 1;
  if (consolidated_density_3()!=0) return 1;
  if (consolidated_density_4()!=0) return 1;
  if (consolidated_density_5()!=0) return 1;
  if (consolidated_density_6()!=0) return 1;
  if (consolidated_density_7()!=0) return 1;
  if (consolidated_density_8()!=0) return 1;
  if (consolidated_density_9()!=0) return 1;
  if (consolidated_density_10()!=0) return 1;
  if (consolidated_density_11()!=0) return 1;
  if (consolidated_density_12()!=0) return 1;
  return 0;
}

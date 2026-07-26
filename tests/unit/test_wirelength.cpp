// Consolidated from test_hpwl_high_degree.cpp
#include "test_wirelength_support.hpp"
#include "../TestSupport.hpp"
static int check(std::size_t n){std::vector<std::pair<double,double>>p;std::vector<placer::LPin>pins;for(std::size_t i=0;i<n;++i){p.push_back({double(i),double((i*7)%n)});pins.push_back({i,0,0});}auto l=wireLevel(p);addWireNet(l,"high",pins);auto a=placer::evaluateWirelength(l),b=placer::evaluateWirelength(l);CHECK_NEAR(a.hpwl,a.paper_l1_value,1e-10);CHECK_NEAR(a.hpwl,b.hpwl,0);CHECK_EQ(a.effective_edge_count_x,2*n-3);CHECK_EQ(a.effective_edge_count_y,2*n-3);return 0;}int consolidated_wirelength_0(){CHECK_EQ(check(4),0);CHECK_EQ(check(5),0);CHECK_EQ(check(10),0);CHECK_EQ(check(1000),0);return 0;}

// Consolidated from test_hpwl_pin_offsets.cpp
#include "test_wirelength_support.hpp"
#include "../TestSupport.hpp"
int consolidated_wirelength_1(){auto l=wireLevel({{10,20},{13,24}});addWireNet(l,"same-cell",{{0,-2,1},{0,3,-4},{1,1,2}});auto e=placer::evaluateWirelength(l);CHECK_NEAR(e.hpwl,16,1e-12);CHECK_NEAR(e.paper_l1_value,16,1e-12);CHECK_EQ(l.nets[0].pins.size(),3u);return 0;}

// Consolidated from test_hpwl_three_pin.cpp
#include "test_wirelength_support.hpp"
#include "../TestSupport.hpp"
int consolidated_wirelength_2(){auto l=wireLevel({{0,2},{4,0},{2,5}});addWireNet(l,"three",{{0,0,0},{1,0,0},{2,0,0}});auto e=placer::evaluateWirelength(l);CHECK_NEAR(e.hpwl,9,1e-12);CHECK_NEAR(e.paper_l1_value,9,1e-12);CHECK_EQ(e.effective_edge_count_x,3u);CHECK_EQ(e.effective_edge_count_y,3u);auto t=wireLevel({{0,0},{0,3},{4,3}});addWireNet(t,"repeat",{{0,0,0},{1,0,0},{2,0,0}});auto q=placer::evaluateWirelength(t);CHECK_NEAR(q.hpwl,7,1e-12);CHECK_NEAR(q.paper_l1_value,7,1e-12);return 0;}

// Consolidated from test_hpwl_ties.cpp
#include "test_wirelength_support.hpp"
#include "../TestSupport.hpp"
#include <algorithm>
int consolidated_wirelength_3(){auto l=wireLevel({{0,0},{0,4},{5,4},{5,2}});addWireNet(l,"extrema",{{0,0,0},{1,0,0},{2,0,0},{3,0,0}});auto a=placer::evaluateWirelength(l);std::reverse(l.nets[0].pins.begin(),l.nets[0].pins.end());auto b=placer::evaluateWirelength(l);CHECK_NEAR(a.hpwl,9,1e-12);CHECK_NEAR(a.hpwl,b.hpwl,0);CHECK_NEAR(a.paper_l1_value,b.paper_l1_value,0);auto all=wireLevel({{1,1},{1,1},{1,1},{1,1},{1,1}});addWireNet(all,"all",{{0,0,0},{1,0,0},{2,0,0},{3,0,0},{4,0,0}});auto t=placer::evaluateWirelength(all);CHECK_NEAR(t.hpwl,0,0);CHECK_NEAR(t.paper_l1_value,0,0);CHECK_EQ(t.effective_edge_count_x,7u);CHECK_EQ(t.effective_edge_count_y,7u);CHECK_EQ(t.tie_count,14u);return 0;}

// Consolidated from test_hpwl_two_pin.cpp
#include "test_wirelength_support.hpp"
#include "../TestSupport.hpp"
int consolidated_wirelength_4(){auto h=wireLevel({{0,0},{5,0}});addWireNet(h,"h",{{0,1,0},{1,-1,0}});CHECK_NEAR(placer::exactHpwl(h),3,1e-12);auto v=wireLevel({{0,0},{0,5}});addWireNet(v,"v",{{0,0,1},{1,0,-2}});CHECK_NEAR(placer::exactHpwl(v),2,1e-12);auto d=wireLevel({{0,0},{5,7}},{false,true});addWireNet(d,"d",{{0,1,-1},{1,-2,2}});auto e=placer::evaluateWirelength(d);CHECK_NEAR(e.hpwl,12,1e-12);CHECK_NEAR(e.paper_l1_value,12,1e-12);CHECK_EQ(e.effective_edge_count_x,1u);CHECK_EQ(e.effective_edge_count_y,1u);CHECK_NEAR(e.gx[1],0,0);CHECK_NEAR(e.gy[1],0,0);auto t=wireLevel({{2,3},{2,3}});addWireNet(t,"t",{{0,0,0},{1,0,0}});CHECK_NEAR(placer::exactHpwl(t),0,0);return 0;}

// Consolidated from test_objective_wire_only.cpp
#include "test_objective_support.hpp"
int consolidated_wirelength_5(){testWireOnly();return 0;}

// Consolidated from test_wire_descent_step.cpp
#include "test_wirelength_support.hpp"
#include "placer/objective/Density.hpp"
#include "../TestSupport.hpp"
int consolidated_wirelength_6(){auto l=wireLevel({{10,10},{20,20},{30,15}},{false,false,true});for(auto&o:l.objects){o.width=1;o.height=1;}addWireNet(l,"descent",{{0,0,0},{1,0,0},{2,0,0}});placer::DensityGrid d({0,100,0,100},1,1,1,1,0);double before=placer::exactHpwl(l),ob=d.evaluate(l).paper_ofr,fx=l.objects[2].x,fy=l.objects[2].y;auto e=placer::evaluateWirelength(l);for(std::size_t i=0;i<2;++i){l.objects[i].x-=0.1*e.gx[i];l.objects[i].y-=0.1*e.gy[i];}double after=placer::exactHpwl(l),oa=d.evaluate(l).paper_ofr;CHECK(after<before);CHECK(oa<=ob);CHECK_NEAR(l.objects[2].x,fx,0);CHECK_NEAR(l.objects[2].y,fy,0);return 0;}

// Consolidated from test_wire_seed_reproducibility.cpp
#include "test_wirelength_support.hpp"
#include "../TestSupport.hpp"
int consolidated_wirelength_7(){auto seq=[](std::uint64_t s){auto l=wireLevel({{2,0},{0,0},{1,0},{3,0}});addWireNet(l,"seed",{{0,0,0},{1,0,0},{2,0,0},{3,0,0}});placer::WirelengthEvaluator e(s);(void)e.evaluate(l);for(auto&o:l.objects)o.x=0;return e.evaluate(l);};auto a=seq(123),b=seq(123),c=seq(124);CHECK_EQ(a.tie_count,b.tie_count);CHECK(a.gx==b.gx);CHECK(a.gy==b.gy);CHECK(a.gx!=c.gx||a.gy!=c.gy);placer::WirelengthEvaluator e(5);auto l=wireLevel({{1,0},{0,0}});addWireNet(l,"r",{{0,0,0},{1,0,0}});(void)e.evaluate(l);l.objects[0].x=0;auto pure=e.evaluate(l,placer::WirelengthMode::PaperL1,false);auto state=e.evaluate(l);CHECK_NEAR(pure.gx[0],1,0);CHECK(state.gx[0]>=0.5&&state.gx[0]<=1);return 0;}

// Consolidated from test_wire_subgradient.cpp
#include "test_wirelength_support.hpp"
#include "../TestSupport.hpp"
#include <array>
int consolidated_wirelength_8(){auto l=wireLevel({{0,1},{3,5}});addWireNet(l,"smooth",{{0,0,0},{1,0,0}});placer::WirelengthEvaluator ev(9);auto e=ev.evaluate(l);double h=1e-6;for(std::size_t id=0;id<2;++id){l.objects[id].x+=h;double fp=placer::exactHpwl(l);l.objects[id].x-=2*h;double fm=placer::exactHpwl(l);l.objects[id].x+=h;CHECK_NEAR(e.gx[id],(fp-fm)/(2*h),1e-8);}auto tie=wireLevel({{0,0},{0,0}});addWireNet(tie,"tie",{{0,0,0},{1,0,0}});placer::WirelengthEvaluator none(1);auto n=none.evaluate(tie);CHECK_NEAR(n.gx[0],1,0);CHECK_NEAR(n.gx[0]+n.gx[1],0,1e-15);for(double z:std::array<double,4>{-0.2,-0.01,0.03,0.4}){auto q=tie;q.objects[0].x+=z;CHECK(placer::exactHpwl(q)+1e-14>=n.hpwl+n.gx[0]*z);}placer::WirelengthEvaluator pos(7);auto p=wireLevel({{1,0},{0,0}});addWireNet(p,"p",{{0,0,0},{1,0,0}});(void)pos.evaluate(p);p.objects[0].x=0;auto pt=pos.evaluate(p);CHECK(pt.gx[0]>=0.5&&pt.gx[0]<=1.0);placer::WirelengthEvaluator neg(7);auto m=wireLevel({{-1,0},{0,0}});addWireNet(m,"m",{{0,0,0},{1,0,0}});(void)neg.evaluate(m);m.objects[0].x=0;auto mt=neg.evaluate(m);CHECK(mt.gx[0]>=-1.0&&mt.gx[0]<=-0.5);auto f=wireLevel({{0,0},{2,0}},{false,true});addWireNet(f,"fixed",{{0,0,0},{1,0,0}});auto fe=placer::evaluateWirelength(f);CHECK_NEAR(fe.gx[0],-1,0);CHECK_NEAR(fe.gx[1],0,0);return 0;}

// Consolidated from test_wire_translation_invariance.cpp
#include "test_wirelength_support.hpp"
#include "../TestSupport.hpp"
#include <numeric>
int consolidated_wirelength_9(){auto l=wireLevel({{-2,1},{3,5},{7,-1},{0,4}});addWireNet(l,"translate",{{0,1,-2},{1,-1,0},{2,0,3},{3,2,-1}});auto a=placer::evaluateWirelength(l);for(auto&o:l.objects){o.x+=123.25;o.y-=91.5;}auto b=placer::evaluateWirelength(l);CHECK_NEAR(a.hpwl,b.hpwl,1e-12);CHECK_NEAR(a.paper_l1_value,b.paper_l1_value,1e-12);CHECK_NEAR(std::accumulate(a.gx.begin(),a.gx.end(),0.0),0,1e-12);CHECK_NEAR(std::accumulate(a.gy.begin(),a.gy.end(),0.0),0,1e-12);return 0;}

// Consolidated from test_wirelength.cpp
#include "placer/io/BookshelfReader.hpp"
#include "placer/objective/Wirelength.hpp"
#include "../TestSupport.hpp"
int consolidated_wirelength_10(){
 auto db=placer::loadBookshelf("tests/data/tiny/tiny_basic/tiny_basic.aux"); auto l=placer::buildLevel0(db);
 auto w=placer::wirelengthSubgradient(l,placer::WirelengthMode::Extrema); CHECK_NEAR(w.hpwl,placer::exactHpwl(l),1e-12);
 placer::Level p; for(int i=0;i<5;++i)p.objects.push_back({std::to_string(i),1,1,double(i),double((i*3)%5),i==4,false});
 placer::LNet net{"five",{}}; for(size_t i=0;i<5;++i)net.pins.push_back({i,0.1*double(i),-0.2*double(i)});p.nets.push_back(net);
 auto paper=placer::wirelengthSubgradient(p,placer::WirelengthMode::PaperL1);
 CHECK_NEAR(paper.hpwl,placer::exactHpwl(p),1e-12);
 CHECK_NEAR(paper.gx[4],0.0,0.0); CHECK_NEAR(paper.gy[4],0.0,0.0);
 // The two x-inner pins have no mutual Equation (6) contribution; the full
 // pair model nevertheless has the same value as independent HPWL.
 CHECK_EQ(paper.gx.size(),p.objects.size());
 return 0;
}

// Consolidated from test_wirelength_properties.cpp
#include "test_wirelength_support.hpp"
#include "../TestSupport.hpp"
#include <algorithm>
#include <random>
int consolidated_wirelength_11(){std::mt19937_64 r(2025);std::uniform_real_distribution<double>d(-20,20);for(std::size_t n=2;n<=20;++n){std::vector<std::pair<double,double>>p;std::vector<bool>f;std::vector<placer::LPin>pins;for(std::size_t i=0;i<n;++i){p.push_back({d(r),d(r)});f.push_back(i%7==0);pins.push_back({i,d(r)/10,d(r)/10});}auto l=wireLevel(p,f);addWireNet(l,"property",pins);auto a=placer::evaluateWirelength(l);CHECK_NEAR(a.hpwl,a.paper_l1_value,1e-10);for(std::size_t i=0;i<n;++i)if(f[i]){CHECK_NEAR(a.gx[i],0,0);CHECK_NEAR(a.gy[i],0,0);}std::shuffle(l.nets[0].pins.begin(),l.nets[0].pins.end(),r);auto b=placer::evaluateWirelength(l);CHECK_NEAR(a.hpwl,b.hpwl,1e-10);CHECK_NEAR(a.paper_l1_value,b.paper_l1_value,1e-10);for(auto&o:l.objects){o.x+=7;o.y-=11;}CHECK_NEAR(placer::exactHpwl(l),a.hpwl,1e-10);CHECK(std::isfinite(a.hpwl)&&std::isfinite(a.paper_l1_value));}return 0;}

int main(){
  if (consolidated_wirelength_0()!=0) return 1;
  if (consolidated_wirelength_1()!=0) return 1;
  if (consolidated_wirelength_2()!=0) return 1;
  if (consolidated_wirelength_3()!=0) return 1;
  if (consolidated_wirelength_4()!=0) return 1;
  if (consolidated_wirelength_5()!=0) return 1;
  if (consolidated_wirelength_6()!=0) return 1;
  if (consolidated_wirelength_7()!=0) return 1;
  if (consolidated_wirelength_8()!=0) return 1;
  if (consolidated_wirelength_9()!=0) return 1;
  if (consolidated_wirelength_10()!=0) return 1;
  if (consolidated_wirelength_11()!=0) return 1;
  return 0;
}

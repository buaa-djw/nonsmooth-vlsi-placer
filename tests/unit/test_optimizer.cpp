// Consolidated from test_optimizer_helpers.cpp
#include "placer/io/BookshelfReader.hpp"
#include "placer/objective/Density.hpp"
#include "placer/optimizer/NonsmoothOptimizer.hpp"
#include "../TestSupport.hpp"
int consolidated_optimizer_0(){ auto db=placer::loadBookshelf("tests/data/tiny/tiny_basic/tiny_basic.aux"); auto l=placer::buildLevel0(db); placer::DensityGrid dg(db.region(),4,4,0.1,0.1); placer::OptimizeConfig cfg; cfg.iterations_per_stage=1; cfg.penalty_stages=1; cfg.level_index=0; placer::GlobalOptimizeState gs{0,std::chrono::steady_clock::now()}; auto r=placer::optimizeLevel(l,db.region(),dg,cfg,gs); CHECK_EQ(r.history.size(),1u); CHECK_EQ(r.history[0].global_iteration,0); CHECK_EQ(r.history[0].no_improve_count,0); CHECK_NEAR(r.history[0].s,.2,1e-15); CHECK_EQ(gs.iteration,1); return 0; }

// Consolidated from test_paper_optimizer_math.cpp
#include "../TestSupport.hpp"
#include "placer/optimizer/NonsmoothOptimizer.hpp"
#include <cmath>
int consolidated_optimizer_1(){
 CHECK_NEAR(placer::paperInitialLambda(10,2),5,1e-12);
 bool threw=false;try{(void)placer::paperInitialLambda(10,0);}catch(...){threw=true;}CHECK(threw);
 CHECK_NEAR(placer::paperStepScale(0),.2,1e-15);CHECK_NEAR(placer::paperStepScale(99),.2,1e-15);
 CHECK_NEAR(placer::paperStepScale(100),.2*2/3,1e-15);CHECK_NEAR(placer::paperStepScale(200),.2*4/9,1e-15);
 CHECK_NEAR(placer::paperStepScale(300),.06,1e-15);CHECK_NEAR(placer::paperStepScale(19999),.06,1e-15);
 CHECK_NEAR(placer::paperStepScale(100,.3,.08),.2,1e-15);CHECK_NEAR(placer::paperStepScale(10000,.3,.08),.08,1e-15);
 std::vector<double> previous{1,2}, current{-1,1};
 CHECK_NEAR(placer::polakRibiereBeta(current,previous),0.2,1e-15); // negative components are not PR+ clipped
 CHECK_NEAR(placer::nextPaperLambda(10,.03,0,false),16,1e-12);
 CHECK_NEAR(placer::nextPaperLambda(10,.1,.3,true),19,1e-12);
 CHECK_NEAR(placer::nextPaperLambda(10,.2,.3,true),22,1e-12);
 CHECK_NEAR(placer::nextPaperLambda(10,.03,0,false,2,3,4),20,1e-12);
 CHECK_EQ(placer::paperNoImprovementLimit(1),1);CHECK_EQ(placer::paperNoImprovementLimit(210904),100);
 return 0;
}

int main(){
  if (consolidated_optimizer_0()!=0) return 1;
  if (consolidated_optimizer_1()!=0) return 1;
  return 0;
}

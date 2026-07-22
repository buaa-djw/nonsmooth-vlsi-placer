#include "placer/io/BookshelfReader.hpp"
#include "placer/objective/Density.hpp"
#include "placer/optimizer/NonsmoothOptimizer.hpp"
#include "../TestSupport.hpp"
int main(){ auto db=placer::loadBookshelf("tests/data/tiny/tiny_basic/tiny_basic.aux"); auto l=placer::buildLevel0(db); placer::DensityGrid dg(db.region(),4,4,1.0,1.0); placer::OptimizeConfig cfg; cfg.iterations_per_stage=1; cfg.penalty_stages=1; cfg.level_index=0; placer::GlobalOptimizeState gs{0,std::chrono::steady_clock::now()}; auto r=placer::optimizeLevel(l,db.region(),dg,cfg,gs); CHECK_EQ(r.history.size(),1u); CHECK_EQ(r.history[0].global_iteration,0); CHECK_EQ(gs.iteration,1); return 0; }

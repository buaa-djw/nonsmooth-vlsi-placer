#include "placer/optimizer/NonsmoothOptimizer.hpp"
#include "placer/multilevel/Projector.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
namespace placer
{
    namespace
    {
        double l1(const std::vector<double> &x, const std::vector<double> &y, const std::vector<size_t> &ids)
        {
            double v = 0;
            for (auto i : ids)
                v += std::abs(x[i]) + std::abs(y[i]);
            return v;
        }

OptimizeResult optimizeLevel(Level &l,const Region&r,const DensityGrid&grid,const OptimizeConfig&cfg,GlobalOptimizeState&gs)
{
    OptimizeResult out;out.level_index=cfg.level_index; const auto ids=l.movableIds(); if(ids.empty())return out;
    auto w=wirelengthSubgradient(l,cfg.mode); auto d=grid.evaluate(l);
    out.initial_hpwl=w.hpwl;out.initial_density_penalty=d.penalty;out.initial_ofr=d.ofr;
    out.wire_gradient_l1=l1(w.gx,w.gy,ids);out.density_gradient_l1=l1(d.gx,d.gy,ids);
    out.lambda=cfg.lambda0>0?cfg.lambda0:paperInitialLambda(out.wire_gradient_l1,out.density_gradient_l1);out.initial_lambda=out.lambda;
    std::vector<double> accepted_x,accepted_y;capture(l,ids,accepted_x,accepted_y);double accepted_ofr=d.ofr;bool have_accepted=false;
    const int stall_limit=cfg.nmax>0?cfg.nmax:paperNoImprovementLimit(ids.size());
    for(int stage=0;stage<cfg.penalty_stages;++stage){
        if(stage>0)out.lambda=nextPaperLambda(out.lambda,accepted_ofr,stage>1?out.stages[out.stages.size()-2].ofr:0.0,stage>1);
        w=wirelengthSubgradient(l,cfg.mode);d=grid.evaluate(l);
        double best=w.hpwl+out.lambda*d.penalty;int best_it=-1,stall=0;std::vector<double> bx,by;capture(l,ids,bx,by);
        std::vector<double> pgx,pgy,pdx,pdy,gx(l.objects.size()),gy(l.objects.size()),dx(l.objects.size()),dy(l.objects.size());
        StageResult sr;sr.stage=stage;sr.lambda=out.lambda;sr.initial_hpwl=w.hpwl;sr.initial_density_penalty=d.penalty;sr.initial_ofr=d.ofr;sr.stop_reason="safety_iteration_limit";
        for(int it=0;it<cfg.iterations_per_stage;++it){
            w=wirelengthSubgradient(l,cfg.mode);d=grid.evaluate(l);
            const double objective=w.hpwl+out.lambda*d.penalty;
            bool isbest=improved(best,objective);if(isbest){best=objective;best_it=it;capture(l,ids,bx,by);stall=0;}else ++stall;
            for(auto i:ids){gx[i]=w.gx[i]+out.lambda*d.gx[i];gy[i]=w.gy[i]+out.lambda*d.gy[i];}
            double beta=0;bool zero_prev=false;if(!pgx.empty()){double den=dot(pgx,pgy,pgx,pgy,ids);if(den<=EPS)zero_prev=true;else{double num=0;for(auto i:ids)num+=gx[i]*(gx[i]-pgx[i])+gy[i]*(gy[i]-pgy[i]);beta=num/den;}}
            for(auto i:ids){dx[i]=-gx[i]+(pdx.empty()?0.0:beta*pdx[i]);dy[i]=-gy[i]+(pdy.empty()?0.0:beta*pdy[i]);}
            double dn=l2(dx,dy,ids),scale=paperStepScale(it),alpha=dn>0?scale*grid.binW()/dn:0;
            HistoryRow row;row.global_iteration=gs.iteration++;row.level=cfg.level_index;row.stage=stage;row.iteration=it;row.lambda=out.lambda;row.raw_hpwl=w.hpwl;row.raw_density_penalty=d.penalty;row.raw_objective=objective;row.ofr=d.ofr;row.wire_gradient_l1=l1(w.gx,w.gy,ids);row.density_gradient_l1=l1(d.gx,d.gy,ids);row.weighted_density_gradient_l1=out.lambda*row.density_gradient_l1;row.total_gradient_l1=l1(gx,gy,ids);row.total_gradient_l2=l2(gx,gy,ids);row.beta_pr=beta;row.direction_norm=dn;row.s=scale;row.bin_width=grid.binW();row.alpha=alpha;row.displacement_norm=alpha*dn;row.overflow_bin_count=d.overflow_bin_count;row.total_overflow=d.total_overflow;row.max_bin_overflow=d.max_bin_overflow;row.max_bin_utilization=d.max_density;row.stage_best_objective=best;row.stage_best_iteration=best_it;row.is_stage_best=isbest;row.no_improve_count=stall;row.restart_due_to_zero_previous_norm=zero_prev;
            sr.iterations=it+1;if(cfg.report_every>0&&row.global_iteration%cfg.report_every==0)std::cout<<"[L"<<cfg.level_index<<" S"<<stage<<" I"<<it<<"] HPWL="<<w.hpwl<<" P="<<d.penalty<<" OFR="<<d.ofr<<" lambda="<<out.lambda<<" s="<<scale<<std::endl;
            if(!std::isfinite(objective)||!std::isfinite(dn)){sr.stop_reason="numeric_error";row.elapsed_sec=std::chrono::duration<double>(std::chrono::steady_clock::now()-gs.start_time).count();out.history.push_back(row);break;}if(dn<=EPS){sr.stop_reason="zero_direction";row.elapsed_sec=std::chrono::duration<double>(std::chrono::steady_clock::now()-gs.start_time).count();out.history.push_back(row);break;}if(stall>=stall_limit){sr.stop_reason="no_objective_improvement";row.elapsed_sec=std::chrono::duration<double>(std::chrono::steady_clock::now()-gs.start_time).count();out.history.push_back(row);break;}
            for(auto i:ids){l.objects[i].x+=alpha*dx[i];l.objects[i].y+=alpha*dy[i];}auto ps=projectLevel(l,r);row.projection_count=ps.projected_objects;row.elapsed_sec=std::chrono::duration<double>(std::chrono::steady_clock::now()-gs.start_time).count();out.history.push_back(row);pgx=gx;pgy=gy;pdx=dx;pdy=dy;
        }
        restore(l,ids,bx,by);w=wirelengthSubgradient(l,cfg.mode);d=grid.evaluate(l);sr.best_iteration=best_it;sr.hpwl=w.hpwl;sr.density_penalty=d.penalty;sr.objective=w.hpwl+out.lambda*d.penalty;sr.ofr=d.ofr;
        if(stage>0&&!improved(accepted_ofr,sr.ofr)){restore(l,ids,accepted_x,accepted_y);sr.stop_reason="ofr_not_improved";out.stages.push_back(sr);out.stop_reason=sr.stop_reason;break;}out.stages.push_back(sr);out.stop_reason=sr.stop_reason;
        accepted_ofr=sr.ofr;capture(l,ids,accepted_x,accepted_y);have_accepted=true;if(sr.ofr<=EPS){out.stop_reason="zero_ofr";out.stages.back().stop_reason="zero_ofr";break;}if(cfg.target_ofr>0.0&&sr.ofr<=cfg.target_ofr)break;
    }
    if(have_accepted)restore(l,ids,accepted_x,accepted_y);w=wirelengthSubgradient(l,cfg.mode);d=grid.evaluate(l);out.hpwl=w.hpwl;out.density_penalty=d.penalty;out.ofr=d.ofr;return out;
}
}

#include "placer/optimizer/NonsmoothOptimizer.hpp"
#include "placer/multilevel/Projector.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
namespace placer
{
    static double rmsIds(const std::vector<double>&x,const std::vector<double>&y,const std::vector<size_t>&ids){ double s=0; for(auto i:ids) s+=x[i]*x[i]+y[i]*y[i]; return ids.empty()?0:std::sqrt(s/(2.0*ids.size())); }
    static double dotIds(const std::vector<double>&ax,const std::vector<double>&ay,const std::vector<double>&bx,const std::vector<double>&by,const std::vector<size_t>&ids){ double s=0; for(auto i:ids) s+=ax[i]*bx[i]+ay[i]*by[i]; return s; }
    OptimizeResult optimizeLevel(Level &l, const Region &r, const DensityGrid &dg, const OptimizeConfig &cfg)
    {
        OptimizeResult out; auto ids=l.movableIds(); auto t0=std::chrono::steady_clock::now();
        auto we=wirelengthSubgradient(l,cfg.mode); auto de=dg.evaluate(l); double wr=rmsIds(we.gx,we.gy,ids), dr=rmsIds(de.gx,de.gy,ids);
        out.lambda = cfg.density_only ? 1.0 : (cfg.lambda0>0 ? cfg.lambda0 : ((dr<EPS||de.penalty<EPS)?1.0:std::min(1e8,std::max(1e-8,cfg.density_gradient_ratio*wr/dr))));
        std::vector<double> gx(l.objects.size()), gy(l.objects.size()), pgx(l.objects.size()), pgy(l.objects.size()), dirx(l.objects.size()), diry(l.objects.size()), bestx(l.objects.size()), besty(l.objects.size());
        for(size_t i=0;i<l.objects.size();++i){ bestx[i]=l.objects[i].x; besty[i]=l.objects[i].y; }
        std::pair<double,double> bestKey{de.ofr_report,we.hpwl}; double bestTotal=we.hpwl+out.lambda*de.penalty; int stall=0, glob=0;
        for(int st=0; st<cfg.penalty_stages; ++st){
            for(int it=0; it<cfg.iterations_per_stage; ++it,++glob){
                we=wirelengthSubgradient(l,cfg.mode); de=dg.evaluate(l);
                for(size_t i=0;i<l.objects.size();++i){ gx[i]=cfg.density_only?de.gx[i]:(we.gx[i]+out.lambda*de.gx[i]); gy[i]=cfg.density_only?de.gy[i]:(we.gy[i]+out.lambda*de.gy[i]); }
                double gr=rmsIds(gx,gy,ids), beta=0; if(glob>0){ std::vector<double> dxg(gx.size()), dyg(gy.size()); for(size_t i=0;i<gx.size();++i){dxg[i]=gx[i]-pgx[i]; dyg[i]=gy[i]-pgy[i];} double den=std::max(EPS,dotIds(pgx,pgy,pgx,pgy,ids)); beta=std::max(0.0,dotIds(gx,gy,dxg,dyg,ids)/den); }
                for(auto i:ids){ dirx[i]=-gx[i]+beta*dirx[i]; diry[i]=-gy[i]+beta*diry[i]; }
                if(dotIds(gx,gy,dirx,diry,ids)>=0){ beta=0; for(auto i:ids){dirx[i]=-gx[i]; diry[i]=-gy[i];} }
                double dn=rmsIds(dirx,diry,ids); double step=std::max(cfg.s_floor,(cfg.s0>0?cfg.s0:std::max(r.xh-r.xl,r.yh-r.yl)/100.0)/(1.0+(double)glob/std::max(EPS,cfg.step_decay)));
                HistoryRow row; row.global_iteration=glob; row.level=cfg.level_index; row.stage=st; row.iteration=it; row.hpwl=we.hpwl; row.density_penalty=de.penalty; row.ofr_penalty=de.ofr_penalty; row.ofr_report=de.ofr_report; row.max_density=de.max_density; row.overflow_bins_penalty=de.overflow_bins_penalty; row.overflow_bins_report=de.overflow_bins_report; row.lambda=out.lambda; row.beta_pr=beta; row.step=step; row.gradient_rms=gr; row.total_norm=dn; row.elapsed_sec=std::chrono::duration<double>(std::chrono::steady_clock::now()-t0).count(); out.history.push_back(row);
                if(cfg.report_every>0 && (it%cfg.report_every==0 || it+1==cfg.iterations_per_stage)) std::cout<<"[optimize] level="<<cfg.level_index<<" stage="<<st<<" iteration="<<it<<" HPWL="<<row.hpwl<<" density="<<row.density_penalty<<" OFR="<<row.ofr_report<<" max_density="<<row.max_density<<" lambda="<<row.lambda<<" step="<<row.step<<" gradient_rms="<<row.gradient_rms<<" elapsed="<<row.elapsed_sec<<std::endl;
                auto key=std::make_pair(de.ofr_report,we.hpwl); double total=we.hpwl+out.lambda*de.penalty; if(key<bestKey || (key==bestKey && total<bestTotal)){ bestKey=key; bestTotal=total; stall=0; for(size_t i=0;i<l.objects.size();++i){ bestx[i]=l.objects[i].x; besty[i]=l.objects[i].y; } } else ++stall;
                if(cfg.target_ofr>0 && de.ofr_report<=cfg.target_ofr) break; if(stall>=cfg.nmax) break; if(dn>EPS){ for(auto i:ids){ l.objects[i].x += step*dirx[i]/dn; l.objects[i].y += step*diry[i]/dn; } projectLevel(l,r); }
                pgx=gx; pgy=gy;
            }
            if(cfg.target_ofr>0 && de.ofr_report<=cfg.target_ofr) break; if(stall>=cfg.nmax) break; if(de.ofr_report>0.20) out.lambda*=cfg.lambda_growth_high; else if(de.ofr_report>0.05) out.lambda*=cfg.lambda_growth_mid; else out.lambda*=cfg.lambda_growth_low;
        }
        for(size_t i=0;i<l.objects.size();++i){ l.objects[i].x=bestx[i]; l.objects[i].y=besty[i]; } projectLevel(l,r); auto fe=dg.evaluate(l); out.hpwl=exactHpwl(l); out.density_penalty=fe.penalty; out.ofr_penalty=fe.ofr_penalty; out.ofr_report=fe.ofr_report; out.max_density=fe.max_density; return out;
    }
}

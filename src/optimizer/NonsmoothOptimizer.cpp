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
        OptimizeResult out;
        auto ids = l.movableIds();
        auto t0 = std::chrono::steady_clock::now();
        if (ids.empty()) return out;
        auto we = wirelengthSubgradient(l, cfg.mode);
        auto de = dg.evaluate(l);
        const double wire_scale = std::max(std::abs(we.hpwl), 1.0);
        const double density_scale = std::max(std::abs(de.penalty), 1.0e-8);
        std::vector<double> wg(we.gx.size()), wh(we.gy.size()), dgx(de.gx.size()), dgy(de.gy.size());
        for (size_t i=0;i<we.gx.size();++i) { wg[i]=we.gx[i]/wire_scale; wh[i]=we.gy[i]/wire_scale; dgx[i]=de.gx[i]/density_scale; dgy[i]=de.gy[i]/density_scale; }
        double wr = rmsIds(wg, wh, ids), dr = rmsIds(dgx, dgy, ids);
        out.lambda = cfg.density_only ? 1.0 : (cfg.lambda0 > 0 ? cfg.lambda0 : ((dr <= EPS || de.penalty <= EPS) ? 1.0 : std::min(1.0e8, std::max(1.0e-8, cfg.density_gradient_ratio * wr / dr))));
        std::vector<double> gx(l.objects.size()), gy(l.objects.size()), pgx, pgy, dirx(l.objects.size()), diry(l.objects.size()), pdx, pdy;
        std::vector<double> bestx(l.objects.size()), besty(l.objects.size());
        for (size_t i=0;i<l.objects.size();++i) { bestx[i]=l.objects[i].x; besty[i]=l.objects[i].y; }
        std::pair<double,double> bestKey{std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()};
        double bestTotal = std::numeric_limits<double>::infinity();
        int stall = 0, level_iteration = 0;
        double previous_stage_ofr = std::numeric_limits<double>::quiet_NaN();
        bool stop_all = false;
        const double default_s0 = cfg.s0 > 0.0 ? cfg.s0 : 0.25 * std::min(dg.binW(), dg.binH());
        const double default_floor = cfg.s_floor > 0.0 ? cfg.s_floor : 1.0e-3 * std::min(dg.binW(), dg.binH());
        for (int st=0; st<cfg.penalty_stages; ++st)
        {
            if (st > 0)
            {
                double current_ofr = de.ofr_report;
                double factor = current_ofr < 0.04 ? cfg.lambda_growth_low : ((!std::isnan(previous_stage_ofr) && current_ofr <= 0.5 * previous_stage_ofr) ? cfg.lambda_growth_mid : cfg.lambda_growth_high);
                if (!std::isnan(previous_stage_ofr) && current_ofr >= previous_stage_ofr * (1.0 - 1.0e-4)) break;
                out.lambda *= factor;
                pgx.clear(); pgy.clear(); pdx.clear(); pdy.clear(); bestTotal = std::numeric_limits<double>::infinity(); stall = 0;
            }
            previous_stage_ofr = de.ofr_report;
            for (int it=0; it<cfg.iterations_per_stage; ++it)
            {
                we = wirelengthSubgradient(l, cfg.mode); de = dg.evaluate(l);
                double total_norm = (cfg.density_only ? 0.0 : we.hpwl / wire_scale) + out.lambda * de.penalty / density_scale;
                auto key = std::make_pair(de.ofr_report, we.hpwl);
                if (key < bestKey) { bestKey = key; for (auto i:ids) { bestx[i]=l.objects[i].x; besty[i]=l.objects[i].y; } }
                double improve_tol = std::isfinite(bestTotal) ? 1.0e-12 * std::max(1.0, std::abs(bestTotal)) : 0.0;
                if (!std::isfinite(bestTotal) || total_norm < bestTotal - improve_tol) { bestTotal = total_norm; stall = 0; } else ++stall;
                for (size_t i=0;i<l.objects.size();++i) { gx[i]=0.0; gy[i]=0.0; }
                for (auto i:ids) { gx[i] = cfg.density_only ? de.gx[i]/density_scale : we.gx[i]/wire_scale + out.lambda*de.gx[i]/density_scale; gy[i] = cfg.density_only ? de.gy[i]/density_scale : we.gy[i]/wire_scale + out.lambda*de.gy[i]/density_scale; }
                double beta = 0.0;
                if (!pgx.empty()) { double num=0; for(auto i:ids) num += gx[i]*(gx[i]-pgx[i]) + gy[i]*(gy[i]-pgy[i]); beta = std::max(0.0, num / std::max(EPS, dotIds(pgx,pgy,pgx,pgy,ids))); }
                for (auto i:ids) { dirx[i] = -gx[i] + (!pdx.empty()?beta*pdx[i]:0.0); diry[i] = -gy[i] + (!pdy.empty()?beta*pdy[i]:0.0); }
                if (dotIds(gx,gy,dirx,diry,ids) >= 0.0) { beta=0.0; for(auto i:ids){dirx[i]=-gx[i]; diry[i]=-gy[i];} }
                double direction_rms = rmsIds(dirx,diry,ids);
                double step = std::max(default_floor, default_s0 / (1.0 + level_iteration / std::max(cfg.step_decay, 1.0)));
                HistoryRow row; row.global_iteration=(int)out.history.size(); row.level=cfg.level_index; row.stage=st; row.iteration=it; row.hpwl=we.hpwl; row.density_penalty=de.penalty; row.ofr_penalty=de.ofr_penalty; row.ofr_report=de.ofr_report; row.max_density=de.max_density; row.overflow_bins_penalty=de.overflow_bins_penalty; row.overflow_bins_report=de.overflow_bins_report; row.lambda=out.lambda; row.beta_pr=beta; row.step=step; row.gradient_rms=rmsIds(gx,gy,ids); row.total_norm=total_norm; row.elapsed_sec=std::chrono::duration<double>(std::chrono::steady_clock::now()-t0).count(); out.history.push_back(row);
                if(cfg.report_every>0 && row.global_iteration%cfg.report_every==0) std::cout<<"[L"<<cfg.level_index<<" S"<<st<<" I"<<it<<"] HPWL="<<row.hpwl<<" Pden="<<row.density_penalty<<" OFR="<<row.ofr_report<<" maxD="<<row.max_density<<" lambda="<<row.lambda<<" step="<<row.step<<std::endl;
                ++level_iteration;
                if ((cfg.nmax > 0 && stall >= cfg.nmax) || direction_rms <= EPS || (de.ofr_report <= cfg.target_ofr && st == cfg.penalty_stages - 1)) { stop_all = true; break; }
                for (auto i:ids) { l.objects[i].x += step*dirx[i]/direction_rms; l.objects[i].y += step*diry[i]/direction_rms; }
                projectLevel(l,r); pgx=gx; pgy=gy; pdx=dirx; pdy=diry;
            }
            if (stop_all) break;
        }
        for (auto i:ids) { l.objects[i].x=bestx[i]; l.objects[i].y=besty[i]; }
        auto fe=dg.evaluate(l); out.hpwl=exactHpwl(l); out.density_penalty=fe.penalty; out.ofr_penalty=fe.ofr_penalty; out.ofr_report=fe.ofr_report; out.max_density=fe.max_density; return out;
    }
}

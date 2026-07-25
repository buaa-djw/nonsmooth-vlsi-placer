#include "placer/Config.hpp"
#include "placer/io/BookshelfReader.hpp"
#include "placer/io/BookshelfWriter.hpp"
#include "placer/io/Reporter.hpp"
#include "placer/multilevel/Clusterer.hpp"
#include "placer/multilevel/Declusterer.hpp"
#include "placer/multilevel/Projector.hpp"
#include "placer/objective/Density.hpp"
#include "placer/optimizer/QuadraticInitializer.hpp"
#include "placer/optimizer/NonsmoothOptimizer.hpp"
#include "placer/postprocess/MacroShifter.hpp"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
int main(int argc, char **argv)
{
    auto t0 = std::chrono::steady_clock::now();
    try
    {
        auto cfg = placer::parseConfig(argc, argv);
        std::filesystem::create_directories(cfg.out);
        placer::writeRunInfoJson(cfg.out / "run_info.json", cfg, 0.0);
        std::cout << "[load] " << cfg.aux.string() << std::endl;
        auto db = placer::loadBookshelf(cfg.aux.string());
        auto region = db.region();
        auto l0 = placer::buildLevel0(db);
        size_t pin_count = 0; for (const auto &n : db.nets) pin_count += n.pin_ids.size();
        std::cout << "[load] cells=" << db.cells.size() << " movable=" << l0.movableIds().size() << " fixed=" << (db.cells.size() - l0.movableIds().size()) << " nets=" << db.nets.size() << " pins=" << pin_count << " rows=" << db.rows.size() << std::endl;
        std::cout << "[hierarchy] begin" << std::endl;
        placer::ClusterConfig cc{cfg.current, cfg.coarsen_ratio, cfg.max_levels, cfg.cluster_degree_cap};
        auto levels = placer::buildHierarchy(l0, cc);
        std::cout << "[hierarchy] levels=" << levels.size() << std::endl;
        placer::writeHierarchyJson(cfg.out / "hierarchy.json", levels);
        std::vector<double> positive_row_heights; for (const auto &row : db.rows) if (row.height > 0.0) positive_row_heights.push_back(row.height); std::sort(positive_row_heights.begin(), positive_row_heights.end()); double row_height = 1.0; if (!positive_row_heights.empty()) { const auto n = positive_row_heights.size(); row_height = n % 2 ? positive_row_heights[n/2] : 0.5 * (positive_row_heights[n/2-1] + positive_row_heights[n/2]); }
        auto &coarsest = levels.back();
        placer::writeLevelPl((cfg.out / "coarsest_before_quadratic.pl").string(), coarsest);
        placer::QuadraticResult quadratic_result;
        if (cfg.quadratic_init)
            { std::cout << "[quadratic] L" << coarsest.index << " movable=" << coarsest.movableIds().size() << " nets=" << coarsest.nets.size() << " iterations=" << cfg.quadratic_iterations << std::endl; quadratic_result = placer::quadraticInitialize(coarsest, region, {cfg.quadratic_iterations, cfg.quadratic_damping, cfg.quadratic_anchor, cfg.quadratic_tolerance, cfg.seed}); }
        else if (placer::needsNullspaceSeed(coarsest))
            placer::seedGrid(coarsest, region, cfg.seed);
        placer::projectLevel(coarsest, region);
        placer::writeLevelPl((cfg.out / "coarsest_after_quadratic.pl").string(), coarsest);
        std::vector<placer::HistoryRow> hist;
        std::vector<placer::OptimizeResult> sums;
        std::vector<placer::InterlevelHpwl> ih;
        int adaptive_current = cfg.current;
        placer::GlobalOptimizeState global_state{0, t0};
        int final_bx=1,final_by=1;
        for (int li = (int)levels.size() - 1; li >= 0; --li)
        {
            auto &lev = levels[(size_t)li];
            if (li + 1 < (int)levels.size())
            {
                auto &coarse = levels[(size_t)li + 1];
                (void)placer::decluster(lev, coarse, region);
                const auto consistency = placer::interlevelHpwlConsistency(coarse, lev);
                if (std::abs(consistency.relative_delta) > cfg.hpwl_continuity_tol) std::cout << "[warning] inter-level HPWL relative delta exceeds " << cfg.hpwl_continuity_tol << std::endl;
                ih.push_back(consistency);
                adaptive_current = std::min(2 * adaptive_current, std::max(1, (int)std::ceil(std::sqrt((double)lev.movableIds().size()))));
            }
            placer::projectLevel(lev, region);
            int bx = cfg.bins_x.value_or(adaptive_current);
            int by = cfg.bins_y.value_or(adaptive_current);
            if(li==0){final_bx=bx;final_by=by;}
            placer::DensityGrid dg(region, bx, by, cfg.penalty_density.value_or(cfg.target_density), cfg.ofr_density.value_or(cfg.target_density),static_cast<unsigned>(cfg.seed));
            placer::OptimizeConfig oc;
            oc.mode = cfg.wirelength_mode;
            oc.iterations_per_stage = cfg.iterations_per_stage;
            oc.penalty_stages = cfg.penalty_stages;
            oc.nmax = cfg.nmax;
            oc.level_index = lev.index;
            oc.density_only = cfg.density_only;
            oc.lambda0 = cfg.lambda0;
            oc.target_ofr = cfg.target_ofr;
            oc.report_every = cfg.report_every;
            std::cout << "[optimize] L" << lev.index << " movable=" << lev.movableIds().size() << " bins=" << bx << "x" << by << std::endl;
            auto res = placer::optimizeLevel(lev, region, dg, oc, global_state);
            if (li > 0 && cfg.macro_shifting)
                (void)placer::macroShifting(lev, region, row_height, cfg.macro_search_rings, cfg.macro_gap);
            hist.insert(hist.end(), res.history.begin(), res.history.end());
            sums.push_back(res);
            placer::writeLevelPl((cfg.out / ("level_" + std::to_string(lev.index) + "_final.pl")).string(), lev);
        }
        placer::writeFinalPl((cfg.out / "final.pl").string(), db, levels.front());
        placer::DensityGrid final_grid(region,final_bx,final_by,cfg.target_density,cfg.target_density,static_cast<unsigned>(cfg.seed));
        const auto db_level=placer::buildLevel0(db);const auto db_density=final_grid.evaluate(db_level);
        auto reloaded_db=placer::loadBookshelf(cfg.aux.string());placer::parsePl((cfg.out/"final.pl").string(),reloaded_db);const auto reloaded_level=placer::buildLevel0(reloaded_db);const auto reloaded_density=final_grid.evaluate(reloaded_level);
        placer::OutputConsistency consistency;consistency.solver_hpwl=sums.back().hpwl;consistency.db_hpwl=placer::exactHpwl(db_level);consistency.reloaded_hpwl=placer::exactHpwl(reloaded_level);consistency.solver_density_penalty=sums.back().density_penalty;consistency.db_density_penalty=db_density.penalty;consistency.reloaded_density_penalty=reloaded_density.penalty;consistency.solver_ofr=sums.back().ofr_report;consistency.db_ofr=db_density.paper_ofr;consistency.reloaded_ofr=reloaded_density.paper_ofr;
        for(size_t i=0;i<db.cells.size();++i)consistency.max_coordinate_difference=std::max({consistency.max_coordinate_difference,std::abs(db.cells[i].x-reloaded_db.cells[i].x),std::abs(db.cells[i].y-reloaded_db.cells[i].y)});
        auto close=[](double a,double b){return std::abs(a-b)<=1e-9*std::max({1.0,std::abs(a),std::abs(b)});};consistency.consistent=close(consistency.solver_hpwl,consistency.db_hpwl)&&close(consistency.db_hpwl,consistency.reloaded_hpwl)&&close(consistency.solver_density_penalty,consistency.db_density_penalty)&&close(consistency.db_density_penalty,consistency.reloaded_density_penalty)&&close(consistency.solver_ofr,consistency.db_ofr)&&close(consistency.db_ofr,consistency.reloaded_ofr)&&consistency.max_coordinate_difference<=1.0e-6+placer::EPS;
        placer::writeHistoryCsv(cfg.out / "history.csv", hist);
        placer::writeInterlevelJson(cfg.out / "interlevel_hpwl.json", ih);
        placer::writeSummaryJson(cfg.out / "summary.json", sums, levels.front(), db, cfg,consistency);
        if(!consistency.consistent)throw std::runtime_error("output round-trip consistency check failed");
        double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
        placer::writeRunInfoJson(cfg.out / "run_info.json", cfg, elapsed);
        std::cout << "[done] " << (cfg.out / "final.pl").string() << std::endl;
        std::cout << "[done] elapsed=" << elapsed << std::endl;
        return 0;
    }
    catch (const std::exception &e)
    {
        std::string m = e.what();
        if (m.rfind("Usage:", 0) == 0)
        {
            std::cout << m;
            return 0;
        }
        std::cerr << "error: " << m << "\n";
        return 2;
    }
}

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
int main(int argc, char **argv)
{
    auto t0 = std::chrono::steady_clock::now();
    try
    {
        auto cfg = placer::parseConfig(argc, argv);
        auto db = placer::loadBookshelf(cfg.aux.string());
        auto region = db.region();
        auto l0 = placer::buildLevel0(db);
        placer::ClusterConfig cc{cfg.current, cfg.coarsen_ratio, cfg.max_levels, cfg.cluster_degree_cap};
        auto levels = placer::buildHierarchy(l0, cc);
        std::filesystem::create_directories(cfg.out);
        placer::writeHierarchyJson(cfg.out / "hierarchy.json", levels);
        auto &coarsest = levels.back();
        placer::writeLevelPl((cfg.out / "coarsest_before_quadratic.pl").string(), coarsest);
        if (cfg.quadratic_init)
            (void)placer::quadraticInitialize(coarsest, region, {cfg.quadratic_iterations, cfg.quadratic_damping, cfg.quadratic_anchor, cfg.quadratic_tolerance, cfg.seed});
        placer::projectLevel(coarsest, region);
        placer::writeLevelPl((cfg.out / "coarsest_after_quadratic.pl").string(), coarsest);
        std::vector<placer::HistoryRow> hist;
        std::vector<placer::OptimizeResult> sums;
        std::vector<placer::InterlevelHpwl> ih;
        for (int li = (int)levels.size() - 1; li >= 0; --li)
        {
            auto &lev = levels[(size_t)li];
            if (li + 1 < (int)levels.size())
            {
                auto &coarse = levels[(size_t)li + 1];
                (void)placer::decluster(lev, coarse, region);
                ih.push_back(placer::interlevelHpwlConsistency(coarse, lev));
            }
            placer::projectLevel(lev, region);
            int bx = cfg.bins_x.value_or(std::max(1, (int)std::sqrt(std::max<size_t>(1, lev.movableIds().size()))));
            int by = cfg.bins_y.value_or(bx);
            placer::DensityGrid dg(region, bx, by, cfg.penalty_density.value_or(cfg.target_density), cfg.ofr_density.value_or(cfg.target_density));
            placer::OptimizeConfig oc;
            oc.mode = cfg.wirelength_mode;
            oc.iterations_per_stage = cfg.iterations_per_stage;
            oc.penalty_stages = cfg.penalty_stages;
            oc.nmax = cfg.nmax;
            oc.level_index = lev.index;
            oc.density_only = cfg.density_only;
            oc.lambda0 = cfg.lambda0;
            oc.density_gradient_ratio = cfg.density_gradient_ratio;
            oc.lambda_growth_high = cfg.lambda_growth_high;
            oc.lambda_growth_mid = cfg.lambda_growth_mid;
            oc.lambda_growth_low = cfg.lambda_growth_low;
            oc.s0 = cfg.s0;
            oc.s_floor = cfg.s_floor;
            oc.step_decay = cfg.step_decay;
            oc.target_ofr = cfg.target_ofr;
            auto res = placer::optimizeLevel(lev, region, dg, oc);
            if (cfg.macro_shifting)
                (void)placer::macroShifting(lev, region, cfg.macro_search_rings, cfg.macro_gap);
            hist.insert(hist.end(), res.history.begin(), res.history.end());
            sums.push_back(res);
            placer::writeLevelPl((cfg.out / ("level_" + std::to_string(lev.index) + "_final.pl")).string(), lev);
        }
        placer::writeFinalPl((cfg.out / "final.pl").string(), db, levels.front());
        placer::writeHistoryCsv(cfg.out / "history.csv", hist);
        placer::writeInterlevelJson(cfg.out / "interlevel_hpwl.json", ih);
        placer::writeSummaryJson(cfg.out / "summary.json", sums, levels.front());
        double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
        placer::writeRunInfoJson(cfg.out / "run_info.json", cfg, elapsed);
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

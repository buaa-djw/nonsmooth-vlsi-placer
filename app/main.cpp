#include "placer/Config.hpp"
#include "placer/io/BookshelfReader.hpp"
#include "placer/io/BookshelfWriter.hpp"
#include "placer/io/Reporter.hpp"
#include "placer/multilevel/Clusterer.hpp"
#include "placer/multilevel/Declusterer.hpp"
#include "placer/multilevel/Projector.hpp"
#include "placer/objective/Density.hpp"
#include "placer/optimizer/NonsmoothOptimizer.hpp"
#include "placer/optimizer/QuadraticInitializer.hpp"
#include "placer/postprocess/MacroShifter.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <vector>
int main(int argc, char **argv) {
  auto t0 = std::chrono::steady_clock::now();
  try {
    auto cfg = placer::parseConfig(argc, argv);
    std::filesystem::create_directories(cfg.out);
    placer::writeRunInfoJson(cfg.out / "run_info.json", cfg, 0.0);
    std::cout << "[load] " << cfg.aux.string() << std::endl;
    auto db = placer::loadBookshelf(cfg.aux.string());
    auto region = db.region();
    auto l0 = placer::buildLevel0(db);
    // Capture benchmark-input metrics before hierarchy construction or
    // quadratic initialization can mutate any coordinates.
    placer::DensityGrid input_grid(region, cfg.bins_x.value_or(cfg.current),
                                   cfg.bins_y.value_or(cfg.current),
                                   cfg.target_density);
    const auto input_density = input_grid.evaluate(l0);
    const placer::InputMetrics input_metrics{
        placer::exactHpwl(l0), input_density.penalty, input_density.ofr};
    size_t pin_count = 0;
    for (const auto &n : db.nets)
      pin_count += n.pin_ids.size();
    std::cout << "[load] cells=" << db.cells.size()
              << " movable=" << l0.movableIds().size()
              << " fixed=" << (db.cells.size() - l0.movableIds().size())
              << " nets=" << db.nets.size() << " pins=" << pin_count
              << " rows=" << db.rows.size() << std::endl;
    std::cout << "[hierarchy] begin" << std::endl;
    placer::ClusterConfig cc{cfg.current, cfg.coarsen_ratio, cfg.max_levels,
                             cfg.cluster_degree_cap};
    auto levels = placer::buildHierarchy(l0, cc);
    std::cout << "[hierarchy] levels=" << levels.size() << std::endl;
    placer::writeHierarchyJson(cfg.out / "hierarchy.json", levels);
    std::vector<double> positive_row_heights;
    for (const auto &row : db.rows)
      if (row.height > 0.0)
        positive_row_heights.push_back(row.height);
    std::sort(positive_row_heights.begin(), positive_row_heights.end());
    double row_height = 1.0;
    if (!positive_row_heights.empty()) {
      const auto n = positive_row_heights.size();
      row_height = n % 2 ? positive_row_heights[n / 2]
                         : 0.5 * (positive_row_heights[n / 2 - 1] +
                                  positive_row_heights[n / 2]);
    }
    auto &coarsest = levels.back();
    placer::writeLevelPl((cfg.out / "coarsest_before_quadratic.pl").string(),
                         coarsest);
    placer::QuadraticResult quadratic_result;
    if (cfg.quadratic_init) {
      std::cout << "[quadratic] L" << coarsest.index
                << " movable=" << coarsest.movableIds().size()
                << " nets=" << coarsest.nets.size()
                << " iterations=" << cfg.quadratic_iterations << std::endl;
      quadratic_result = placer::quadraticInitialize(
          coarsest, region,
          {cfg.quadratic_iterations, cfg.quadratic_damping,
           cfg.quadratic_anchor, cfg.quadratic_tolerance, cfg.seed});
    } else if (placer::needsNullspaceSeed(coarsest))
      placer::seedGrid(coarsest, region, cfg.seed);
    (void)placer::projectLevel(coarsest, region);
    placer::writeLevelPl((cfg.out / "coarsest_after_quadratic.pl").string(),
                         coarsest);
    std::vector<placer::HistoryRow> hist;
    std::vector<placer::OptimizeResult> sums;
    std::vector<placer::InterlevelHpwl> ih;
    int adaptive_current = cfg.current;
    placer::GlobalOptimizeState global_state{0, t0};
    int final_bx = cfg.current, final_by = cfg.current;
    for (int li = (int)levels.size() - 1; li >= 0; --li) {
      auto &lev = levels[(size_t)li];
      if (li + 1 < (int)levels.size()) {
        auto &coarse = levels[(size_t)li + 1];
        (void)placer::decluster(lev, coarse, region);
        const auto consistency = placer::interlevelHpwlConsistency(coarse, lev);
        if (std::abs(consistency.relative_delta) > cfg.hpwl_continuity_tol)
          std::cout << "[warning] inter-level HPWL relative delta exceeds "
                    << cfg.hpwl_continuity_tol << std::endl;
        ih.push_back(consistency);
        adaptive_current = std::min(
            2 * adaptive_current,
            std::max(
                1, (int)std::ceil(std::sqrt((double)lev.movableIds().size()))));
      }
      (void)placer::projectLevel(lev, region);
      int bx = cfg.bins_x.value_or(adaptive_current);
      int by = cfg.bins_y.value_or(adaptive_current);
      if (li == 0) {
        final_bx = bx;
        final_by = by;
      }
      placer::DensityGrid dg(region, bx, by, cfg.target_density);
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
      std::cout << "[optimize] L" << lev.index
                << " movable=" << lev.movableIds().size() << " bins=" << bx
                << "x" << by << std::endl;
      auto res = placer::optimizeLevel(lev, region, dg, oc, global_state);
      if (li > 0 && cfg.macro_shifting)
        (void)placer::macroShifting(lev, region, row_height,
                                    cfg.macro_search_rings, cfg.macro_gap);
      hist.insert(hist.end(), res.history.begin(), res.history.end());
      sums.push_back(res);
      placer::writeLevelPl(
          (cfg.out / ("level_" + std::to_string(lev.index) + "_final.pl"))
              .string(),
          lev);
    }
    // Writing synchronizes the authoritative fine Level into DB. Reloading the
    // emitted file into a copy makes consistency a measured round trip.
    placer::writeFinalPl((cfg.out / "final.pl").string(), db, levels.front());
    const auto database_level = placer::buildLevel0(db);
    placer::DensityGrid final_grid(region, final_bx, final_by,
                                   cfg.target_density);
    const auto database_density = final_grid.evaluate(database_level);
    auto reloaded_db = db;
    placer::parsePl((cfg.out / "final.pl").string(), reloaded_db);
    const auto reloaded_level = placer::buildLevel0(reloaded_db);
    const auto reloaded_density = final_grid.evaluate(reloaded_level);
    placer::OutputConsistencyReport consistency;
    consistency.solver_hpwl = sums.back().hpwl;
    consistency.database_hpwl = placer::exactHpwl(database_level);
    consistency.reloaded_hpwl = placer::exactHpwl(reloaded_level);
    consistency.solver_ofr = sums.back().ofr;
    consistency.database_ofr = database_density.ofr;
    consistency.reloaded_ofr = reloaded_density.ofr;
    for (size_t i = 0; i < db.cells.size(); ++i)
      consistency.max_coordinate_difference =
          std::max(consistency.max_coordinate_difference,
                   std::max(std::abs(db.cells[i].x - reloaded_db.cells[i].x),
                            std::abs(db.cells[i].y - reloaded_db.cells[i].y)));
    auto close = [](double a, double b) {
      return std::abs(a - b) <= 1e-9 * std::max(1.0, std::abs(a));
    };
    // Compare evaluated metrics as well as coordinates: this detects stale DB
    // state and parser/write-back errors that plausible coordinates can hide.
    consistency.consistent =
        close(consistency.solver_hpwl, consistency.database_hpwl) &&
        close(consistency.database_hpwl, consistency.reloaded_hpwl) &&
        close(consistency.solver_ofr, consistency.database_ofr) &&
        close(consistency.database_ofr, consistency.reloaded_ofr);
    placer::writeHistoryCsv(cfg.out / "history.csv", hist);
    placer::writeInterlevelJson(cfg.out / "interlevel_hpwl.json", ih);
    placer::writeSummaryJson(cfg.out / "summary.json", sums, db, cfg,
                             input_metrics, consistency);
    double elapsed =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - t0)
            .count();
    placer::writeRunInfoJson(cfg.out / "run_info.json", cfg, elapsed);
    std::cout << "[done] " << (cfg.out / "final.pl").string() << std::endl;
    std::cout << "[done] elapsed=" << elapsed << std::endl;
    if (!consistency.consistent) {
      std::cerr << "error: final placement output consistency check failed\n";
      return 3;
    }
    return 0;
  } catch (const std::exception &e) {
    std::string m = e.what();
    if (m.rfind("Usage:", 0) == 0) {
      std::cout << m;
      return 0;
    }
    std::cerr << "error: " << m << "\n";
    return 2;
  }
}

#pragma once
#include "placer/multilevel/Clusterer.hpp"
#include "placer/optimizer/NonsmoothOptimizer.hpp"
#include "placer/objective/Wirelength.hpp"
#include <filesystem>
#include <string>
#include <vector>
namespace placer
{
    struct OutputConsistency { double solver_hpwl{},db_hpwl{},reloaded_hpwl{},solver_density_penalty{},db_density_penalty{},reloaded_density_penalty{},solver_ofr{},db_ofr{},reloaded_ofr{},max_coordinate_difference{}; bool consistent{}; };
    [[nodiscard]] std::vector<std::string> historyFields();
    void writeHistoryCsv(const std::filesystem::path &, const std::vector<HistoryRow> &);
    void writeHierarchyJson(const std::filesystem::path &, const std::vector<Level> &);
    void writeInterlevelJson(const std::filesystem::path &, const std::vector<InterlevelHpwl> &);
    void writeSummaryJson(const std::filesystem::path &, const std::vector<OptimizeResult> &, const Level &, const PlacementDB &, const Config &, const OutputConsistency &);
    void writeRunInfoJson(const std::filesystem::path &, const Config &, double elapsed);
}

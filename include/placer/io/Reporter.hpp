#pragma once
#include "placer/multilevel/Clusterer.hpp"
#include "placer/optimizer/NonsmoothOptimizer.hpp"
#include "placer/objective/Wirelength.hpp"
#include <filesystem>
#include <string>
#include <vector>
namespace placer
{
    struct OutputConsistencyReport
    {
        double solver_hpwl{}, database_hpwl{}, reloaded_hpwl{};
        double solver_ofr{}, database_ofr{}, reloaded_ofr{};
        double max_coordinate_difference{};
        bool consistent{};
    };
    struct InputMetrics { double hpwl{}, density_penalty{}, ofr{}; };
    [[nodiscard]] std::vector<std::string> historyFields();
    void writeHistoryCsv(const std::filesystem::path &, const std::vector<HistoryRow> &);
    void writeHierarchyJson(const std::filesystem::path &, const std::vector<Level> &);
    void writeInterlevelJson(const std::filesystem::path &, const std::vector<InterlevelHpwl> &);
    void writeSummaryJson(const std::filesystem::path &, const std::vector<OptimizeResult> &, const PlacementDB &, const Config &, const InputMetrics &, const OutputConsistencyReport &);
    void writeRunInfoJson(const std::filesystem::path &, const Config &, double elapsed);
}

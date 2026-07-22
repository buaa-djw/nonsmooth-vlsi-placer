#pragma once
#include "placer/multilevel/Clusterer.hpp"
#include "placer/optimizer/NonsmoothOptimizer.hpp"
#include "placer/objective/Wirelength.hpp"
#include <filesystem>
#include <string>
#include <vector>
namespace placer
{
    [[nodiscard]] std::vector<std::string> historyFields();
    void writeHistoryCsv(const std::filesystem::path &, const std::vector<HistoryRow> &);
    void writeHierarchyJson(const std::filesystem::path &, const std::vector<Level> &);
    void writeInterlevelJson(const std::filesystem::path &, const std::vector<InterlevelHpwl> &);
    void writeSummaryJson(const std::filesystem::path &, const std::vector<OptimizeResult> &, const Level &);
    void writeRunInfoJson(const std::filesystem::path &, const Config &, double elapsed);
}

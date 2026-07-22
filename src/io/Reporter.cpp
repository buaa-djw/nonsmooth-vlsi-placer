#include "placer/io/Reporter.hpp"
#include <fstream>
#include <iomanip>
namespace placer
{
    std::vector<std::string> historyFields() { return {"global_iteration", "level", "stage", "iteration", "hpwl", "density_penalty", "ofr_penalty", "ofr_report", "max_density", "overflow_bins_penalty", "overflow_bins_report", "lambda", "beta_pr", "step", "gradient_rms", "total_norm", "elapsed_sec"}; }
    static std::string esc(const std::string &s)
    {
        std::string r = "\"";
        for (char c : s)
        {
            if (c == '\"' || c == '\\')
                r += '\\';
            r += c;
        }
        return r + '\"';
    }
    void writeHistoryCsv(const std::filesystem::path &p, const std::vector<HistoryRow> &h)
    {
        std::ofstream f(p);
        auto fs = historyFields();
        for (size_t i = 0; i < fs.size(); ++i)
            f << (i ? "," : "") << fs[i];
        f << "\n"
          << std::setprecision(17);
        for (auto &r : h)
            f << r.global_iteration << ',' << r.level << ',' << r.stage << ',' << r.iteration << ',' << r.hpwl << ',' << r.density_penalty << ',' << r.ofr_penalty << ',' << r.ofr_report << ',' << r.max_density << ',' << r.overflow_bins_penalty << ',' << r.overflow_bins_report << ',' << r.lambda << ',' << r.beta_pr << ',' << r.step << ',' << r.gradient_rms << ',' << r.total_norm << ',' << r.elapsed_sec << "\n";
    }
    void writeHierarchyJson(const std::filesystem::path &p, const std::vector<Level> &hs)
    {
        std::ofstream f(p);
        f << "{\"levels\":[";
        for (size_t i = 0; i < hs.size(); ++i)
        {
            auto &l = hs[i];
            f << (i ? "," : "") << "{\"index\":" << l.index << ",\"objects\":" << l.objects.size() << ",\"movable\":" << l.movableIds().size() << ",\"macros\":" << l.macroIds().size() << ",\"nets\":" << l.nets.size() << "}";
        }
        f << "]}\n";
    }
    void writeInterlevelJson(const std::filesystem::path &p, const std::vector<InterlevelHpwl> &v)
    {
        std::ofstream f(p);
        f << "[";
        for (size_t i = 0; i < v.size(); ++i)
        {
            auto &r = v[i];
            f << (i ? "," : "") << "{\"coarse_hpwl\":" << r.coarse_hpwl << ",\"fine_hpwl\":" << r.fine_hpwl << ",\"delta\":" << r.delta << ",\"relative_delta\":" << r.relative_delta << ",\"ratio\":" << r.ratio << ",\"coarse_nets\":" << r.coarse_nets << ",\"fine_nets\":" << r.fine_nets << ",\"paired_nets\":" << r.paired_nets << ",\"sum_abs_net_delta\":" << r.sum_abs_net_delta << ",\"max_abs_net_delta\":" << r.max_abs_net_delta << "}";
        }
        f << "]\n";
    }
    void writeSummaryJson(const std::filesystem::path &p, const std::vector<OptimizeResult> &rs, const Level &l)
    {
        std::ofstream f(p);
        f << "{\"final_hpwl\":" << exactHpwl(l) << ",\"levels\":[";
        for (size_t i = 0; i < rs.size(); ++i)
        {
            auto &r = rs[i];
            f << (i ? "," : "") << "{\"hpwl\":" << r.hpwl << ",\"density_penalty\":" << r.density_penalty << ",\"ofr_penalty\":" << r.ofr_penalty << ",\"ofr_report\":" << r.ofr_report << ",\"max_density\":" << r.max_density << ",\"lambda\":" << r.lambda << "}";
        }
        f << "]}\n";
    }
    void writeRunInfoJson(const std::filesystem::path &p, const Config &c, double e)
    {
        std::ofstream f(p);
        f << "{\"aux\":" << esc(c.aux.string()) << ",\"out\":" << esc(c.out.string()) << ",\"wirelength_mode\":" << esc(toString(c.wirelength_mode)) << ",\"elapsed_sec\":" << e << ",\"pure_cpp\":true}\n";
    }
}

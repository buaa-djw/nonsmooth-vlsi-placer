#pragma once
#include <filesystem>
#include <optional>
#include <string>
namespace placer
{
    enum class WirelengthMode
    {
        PaperL1,
        B2B,
        Extrema
    };
    struct Config
    {
        std::filesystem::path aux;
        std::optional<std::string> expected_benchmark;
        std::filesystem::path out{"output/nonsmooth_single"};
        WirelengthMode wirelength_mode{WirelengthMode::PaperL1};
        double target_density{1.0};

        std::optional<int> bins_x;
        std::optional<int> bins_y;
        int current{150};
        double coarsen_ratio{5.0};
        int max_levels{16};
        int cluster_degree_cap{256};
        bool quadratic_init{true};
        int quadratic_iterations{200};
        double quadratic_damping{0.75};
        double quadratic_anchor{1.0e-4};
        double quadratic_tolerance{1.0e-3};
        int iterations_per_stage{10000};
        int penalty_stages{20};
        bool density_only{false};
        double lambda0{0.0};


        double target_ofr{0.0};
        int report_every{10};
        int nmax{0};
        double hpwl_continuity_tol{1.0e-8};
        bool macro_shifting{true};
        int macro_search_rings{30};
        double macro_gap{0.0};
        bool whitespace_allocation{false};
        int wsa_leaf_size{64};
        double wsa_min_fraction{0.05};
        int seed{1};
    };
    [[nodiscard]] Config parseConfig(int argc, char **argv);
    [[nodiscard]] std::string helpText();
    [[nodiscard]] std::string toString(WirelengthMode m);
}

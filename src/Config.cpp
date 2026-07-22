#include "placer/Config.hpp"
#include <cstdlib>
#include <stdexcept>
#include <sstream>
#include <optional>
namespace placer
{
    namespace
    {
        std::string need(int &i, int argc, char **argv, const std::string &o)
        {
            if (i + 1 >= argc)
                throw std::runtime_error(o + " requires an argument");
            return argv[++i];
        }
        int toi(const std::string &s, const std::string &o)
        {
            size_t p = 0;
            try
            {
                int v = std::stoi(s, &p);
                if (p != s.size())
                    throw std::invalid_argument("x");
                return v;
            }
            catch (const std::exception &)
            {
                throw std::runtime_error("invalid integer for " + o + ": " + s);
            }
        }
        double tod(const std::string &s, const std::string &o)
        {
            size_t p = 0;
            try
            {
                double v = std::stod(s, &p);
                if (p != s.size())
                    throw std::invalid_argument("x");
                return v;
            }
            catch (const std::exception &)
            {
                throw std::runtime_error("invalid number for " + o + ": " + s);
            }
        }
    }
    std::string toString(WirelengthMode m) { return m == WirelengthMode::PaperL1 ? "paper_l1" : m == WirelengthMode::B2B ? "b2b"
                                                                                                                         : "extrema"; }
    std::string helpText() { return "Usage: nonsmooth_placer AUX [options]\n--out DIR --wirelength-mode paper_l1|b2b|extrema --target-density V --penalty-density V --ofr-density V --bins NX NY --current N --coarsen-ratio V --max-levels N --cluster-degree-cap N --quadratic-init --no-quadratic-init --quadratic-iterations N --quadratic-damping V --quadratic-anchor V --quadratic-tolerance V --iterations N --iterations-per-stage N --penalty-stages N --density-only --no-density-only --lambda0 V --density-gradient-ratio V --lambda-growth-high V --lambda-growth-mid V --lambda-growth-low V --s0 V --s-floor V --step-decay V --target-ofr V --report-every N --nmax N --hpwl-continuity-tol V --macro-shifting --no-macro-shifting --macro-search-rings N --macro-gap V --whitespace-allocation --no-whitespace-allocation --wsa-leaf-size N --wsa-min-fraction V --seed N\n"; }
    Config parseConfig(int argc, char **argv)
    {
        Config c;
        std::optional<int> compatibility_iterations;
        if (argc <= 1)
            throw std::runtime_error("missing aux");
        for (int i = 1; i < argc; ++i)
        {
            std::string a = argv[i];
            if (a == "--help" || a == "-h")
            {
                throw std::runtime_error(helpText());
            }
            else if (a == "--out")
                c.out = need(i, argc, argv, a);
            else if (a == "--wirelength-mode")
            {
                auto v = need(i, argc, argv, a);
                if (v == "paper_l1")
                    c.wirelength_mode = WirelengthMode::PaperL1;
                else if (v == "b2b")
                    c.wirelength_mode = WirelengthMode::B2B;
                else if (v == "extrema")
                    c.wirelength_mode = WirelengthMode::Extrema;
                else
                    throw std::runtime_error("invalid --wirelength-mode");
            }
            else if (a == "--target-density")
                c.target_density = tod(need(i, argc, argv, a), a);
            else if (a == "--penalty-density")
                c.penalty_density = tod(need(i, argc, argv, a), a);
            else if (a == "--ofr-density")
                c.ofr_density = tod(need(i, argc, argv, a), a);
            else if (a == "--bins")
            {
                c.bins_x = toi(need(i, argc, argv, a), a);
                c.bins_y = toi(need(i, argc, argv, a), a);
            }
            else if (a == "--current")
                c.current = toi(need(i, argc, argv, a), a);
            else if (a == "--coarsen-ratio")
                c.coarsen_ratio = tod(need(i, argc, argv, a), a);
            else if (a == "--max-levels")
                c.max_levels = toi(need(i, argc, argv, a), a);
            else if (a == "--cluster-degree-cap")
                c.cluster_degree_cap = toi(need(i, argc, argv, a), a);
            else if (a == "--quadratic-init")
                c.quadratic_init = true;
            else if (a == "--no-quadratic-init")
                c.quadratic_init = false;
            else if (a == "--quadratic-iterations")
                c.quadratic_iterations = toi(need(i, argc, argv, a), a);
            else if (a == "--quadratic-damping")
                c.quadratic_damping = tod(need(i, argc, argv, a), a);
            else if (a == "--quadratic-anchor")
                c.quadratic_anchor = tod(need(i, argc, argv, a), a);
            else if (a == "--quadratic-tolerance")
                c.quadratic_tolerance = tod(need(i, argc, argv, a), a);
            else if (a == "--iterations")
                compatibility_iterations = toi(need(i, argc, argv, a), a);
            else if (a == "--iterations-per-stage")
                c.iterations_per_stage = toi(need(i, argc, argv, a), a);
            else if (a == "--penalty-stages")
                c.penalty_stages = toi(need(i, argc, argv, a), a);
            else if (a == "--density-only")
                c.density_only = true;
            else if (a == "--no-density-only")
                c.density_only = false;
            else if (a == "--lambda0")
                c.lambda0 = tod(need(i, argc, argv, a), a);
            else if (a == "--density-gradient-ratio")
                c.density_gradient_ratio = tod(need(i, argc, argv, a), a);
            else if (a == "--lambda-growth-high")
                c.lambda_growth_high = tod(need(i, argc, argv, a), a);
            else if (a == "--lambda-growth-mid")
                c.lambda_growth_mid = tod(need(i, argc, argv, a), a);
            else if (a == "--lambda-growth-low")
                c.lambda_growth_low = tod(need(i, argc, argv, a), a);
            else if (a == "--s0")
                c.s0 = tod(need(i, argc, argv, a), a);
            else if (a == "--s-floor")
                c.s_floor = tod(need(i, argc, argv, a), a);
            else if (a == "--step-decay")
                c.step_decay = tod(need(i, argc, argv, a), a);
            else if (a == "--target-ofr")
                c.target_ofr = tod(need(i, argc, argv, a), a);
            else if (a == "--report-every")
                c.report_every = toi(need(i, argc, argv, a), a);
            else if (a == "--nmax")
                c.nmax = toi(need(i, argc, argv, a), a);
            else if (a == "--hpwl-continuity-tol")
                c.hpwl_continuity_tol = tod(need(i, argc, argv, a), a);
            else if (a == "--macro-shifting")
                c.macro_shifting = true;
            else if (a == "--no-macro-shifting")
                c.macro_shifting = false;
            else if (a == "--macro-search-rings")
                c.macro_search_rings = toi(need(i, argc, argv, a), a);
            else if (a == "--macro-gap")
                c.macro_gap = tod(need(i, argc, argv, a), a);
            else if (a == "--whitespace-allocation")
                c.whitespace_allocation = true;
            else if (a == "--no-whitespace-allocation")
                c.whitespace_allocation = false;
            else if (a == "--wsa-leaf-size")
                c.wsa_leaf_size = toi(need(i, argc, argv, a), a);
            else if (a == "--wsa-min-fraction")
                c.wsa_min_fraction = tod(need(i, argc, argv, a), a);
            else if (a == "--seed")
                c.seed = toi(need(i, argc, argv, a), a);
            else if (a.rfind("--", 0) == 0)
                throw std::runtime_error("unknown option " + a);
            else if (c.aux.empty())
                c.aux = a;
            else
                throw std::runtime_error("unexpected positional argument " + a);
        }
        if (compatibility_iterations.has_value()) c.iterations_per_stage = *compatibility_iterations;
        if (c.aux.empty())
            throw std::runtime_error("missing aux");
        if (!(0.0 < c.target_density && c.target_density <= 1.0))
            throw std::runtime_error("--target-density must be in (0,1]");
        if (c.penalty_density && !(0.0 < *c.penalty_density && *c.penalty_density <= 1.0))
            throw std::runtime_error("--penalty-density must be in (0,1]");
        if (c.ofr_density && !(0.0 < *c.ofr_density && *c.ofr_density <= 1.0))
            throw std::runtime_error("--ofr-density must be in (0,1]");
        if (c.current <= 0 || c.cluster_degree_cap < 2)
            throw std::runtime_error("--current must be positive and --cluster-degree-cap >= 2");
        if (c.coarsen_ratio <= 1.0)
            throw std::runtime_error("--coarsen-ratio must be > 1");
        if (c.iterations_per_stage < 0 || c.penalty_stages <= 0)
            throw std::runtime_error("iteration counts must be nonnegative/positive");
        if (c.hpwl_continuity_tol < 0.0)
            throw std::runtime_error("--hpwl-continuity-tol must be nonnegative");
        if (c.lambda_growth_low <= 0 || c.lambda_growth_mid <= 0 || c.lambda_growth_high <= 0)
            throw std::runtime_error("lambda growth factors must be positive");
        if (!(0.0 < c.quadratic_damping && c.quadratic_damping <= 1.0))
            throw std::runtime_error("--quadratic-damping must be in (0,1]");
        if (!(0.0 < c.wsa_min_fraction && c.wsa_min_fraction < 0.5))
            throw std::runtime_error("--wsa-min-fraction must be in (0,0.5)");
        return c;
    }
}

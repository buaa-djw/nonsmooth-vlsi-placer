#include "placer/objective/Objective.hpp"
#include <cmath>
#include <stdexcept>
#include <string>

namespace placer
{
namespace
{
void requireFinite(double value, const char *field)
{
    if (!std::isfinite(value)) throw std::runtime_error(std::string("objective evaluator: non-finite ") + field);
}
void norms(const std::vector<double> &x, const std::vector<double> &y, double &l1, double &l2)
{
    if (x.size() != y.size()) throw std::runtime_error("objective evaluator: gradient vector size mismatch");
    double square = 0.0;
    for (std::size_t i = 0; i < x.size(); ++i) {
        requireFinite(x[i], "gradient component"); requireFinite(y[i], "gradient component");
        l1 += std::abs(x[i]) + std::abs(y[i]); square += x[i]*x[i] + y[i]*y[i];
    }
    l2 = std::sqrt(square); requireFinite(l1, "gradient L1 norm"); requireFinite(l2, "gradient L2 norm");
}
}

ObjectiveEvaluator::ObjectiveEvaluator(std::uint64_t wire_seed) : wire_(wire_seed) {}
void ObjectiveEvaluator::reset(std::uint64_t wire_seed) { wire_.reset(wire_seed); calls_ = {}; }

ObjectiveEvaluation ObjectiveEvaluator::evaluate(const Level &level, const DensityGrid &grid, double lambda,
                                                  ObjectiveEvaluationMode mode, WirelengthMode wire_mode)
{
    if (!std::isfinite(lambda) || lambda < 0.0)
        throw std::runtime_error("objective evaluator: lambda must be finite and nonnegative (lambda=" + std::to_string(lambda) + ")");
    const bool stateful = mode == ObjectiveEvaluationMode::StatefulGradient;
    WireEval wire;
    if (stateful) {
        wire = wire_.evaluate(level, wire_mode, true); ++calls_.stateful_evaluations; ++calls_.wire_state_commits;
    } else {
        // A copy makes report/value evaluation pure: neither RNG nor previous-pin
        // history in the optimizer-owned evaluator can advance.
        auto preview = wire_; wire = preview.evaluate(level, wire_mode, false); ++calls_.value_evaluations;
    }
    const auto density = grid.evaluate(level); ++calls_.density_evaluations;
    ObjectiveEvaluation result;
    result.exact_hpwl = wire.hpwl; result.paper_l1_value = wire.paper_l1_value;
    result.density_penalty = density.quadratic_penalty; result.paper_ofr = density.paper_ofr;
    result.total_overflow=density.total_overflow; result.maximum_bin_overflow=density.max_bin_overflow; result.maximum_bin_density=density.maximum_bin_density;
    result.lambda = lambda;
    // Paper Eq. (12): F = W + lambda P. OFR is a report/stopping metric only.
    result.objective = result.paper_l1_value + lambda * result.density_penalty;
    result.wire_tie_count = wire.tie_count; result.overflow_bin_count = density.overflow_bin_count;
    requireFinite(result.objective, "combined objective");
    if (!stateful) return result;
    const auto count = level.objects.size();
    if (wire.gx.size()!=count || wire.gy.size()!=count || density.gx.size()!=count || density.gy.size()!=count)
        throw std::runtime_error("objective evaluator: component gradient size does not match level objects");
    result.wire_grad_x=wire.gx; result.wire_grad_y=wire.gy; result.density_grad_x=density.gx; result.density_grad_y=density.gy;
    result.grad_x.resize(count); result.grad_y.resize(count);
    for(std::size_t i=0;i<count;++i){
        // Complete paper subgradient: g = g_W + lambda g_P, without scaling,
        // normalization, clipping, or a hidden coefficient.
        result.grad_x[i]=wire.gx[i]+lambda*density.gx[i]; result.grad_y[i]=wire.gy[i]+lambda*density.gy[i];
        requireFinite(result.grad_x[i], "combined x gradient"); requireFinite(result.grad_y[i], "combined y gradient");
        if(level.objects[i].fixed && (wire.gx[i]!=0.0 || wire.gy[i]!=0.0 || density.gx[i]!=0.0 || density.gy[i]!=0.0 || result.grad_x[i]!=0.0 || result.grad_y[i]!=0.0))
            throw std::runtime_error("objective evaluator: fixed object has nonzero component gradient");
    }
    norms(result.wire_grad_x,result.wire_grad_y,result.wire_grad_l1,result.wire_grad_l2);
    norms(result.density_grad_x,result.density_grad_y,result.density_grad_l1,result.density_grad_l2);
    norms(result.grad_x,result.grad_y,result.combined_grad_l1,result.combined_grad_l2);
    return result;
}
}

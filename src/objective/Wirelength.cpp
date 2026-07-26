#include "placer/objective/Wirelength.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>

namespace placer
{
namespace
{
constexpr double TIE_RELATIVE_TOLERANCE = 1.0e-14;
double tieTolerance(double a, double b) { return TIE_RELATIVE_TOLERANCE * std::max({1.0, std::abs(a), std::abs(b)}); }
bool tied(double a, double b) { return std::abs(a - b) <= tieTolerance(a, b); }
void requireFinite(double value, const std::string &what) { if (!std::isfinite(value)) throw std::runtime_error("non-finite wirelength " + what); }

struct AxisResult { double value{}; std::size_t edges{}, ties{}; };

template <typename Previous>
double edgeSubgradient(double a, double b, const Previous &previous, std::mt19937_64 &random, std::size_t &tie_count)
{
    // Paper Eq. (13): sign away from a tie; at a tie use cos(theta), with
    // theta selected from the interval implied by the previous pin ordering.
    if (!tied(a, b)) return a > b ? 1.0 : -1.0;
    ++tie_count;
    double theta = 0.0;
    if (previous) {
        const double pa = previous->first, pb = previous->second;
        if (!tied(pa, pb)) {
            const double pi = std::acos(-1.0);
            const auto bounds = pa > pb ? std::pair<double,double>{0.0, pi / 3.0} : std::pair<double,double>{2.0 * pi / 3.0, pi};
            theta = std::uniform_real_distribution<double>(bounds.first, bounds.second)(random);
        }
    }
    requireFinite(theta, "tie theta");
    const double value = std::cos(theta);
    requireFinite(value, "tie cosine");
    return value;
}

template <typename PreviousAt>
AxisResult paperAxis(const Level &level, const LNet &net, const std::vector<double> &values,
                     std::vector<double> &gradient, std::mt19937_64 &random, PreviousAt previous_at)
{
    const std::size_t degree = values.size();
    AxisResult result;
    if (degree <= 1) return result;
    auto add = [&](std::size_t i, std::size_t j, double weight) {
        result.value += weight * std::abs(values[i] - values[j]);
        const double subgradient = edgeSubgradient(values[i], values[j], previous_at(i, j), random, result.ties);
        const auto oi = net.pins[i].object_id, oj = net.pins[j].object_id;
        if (!level.objects[oi].fixed) gradient[oi] += weight * subgradient;
        if (!level.objects[oj].fixed) gradient[oj] -= weight * subgradient;
        ++result.edges;
    };
    if (degree == 2) {
        add(0, 1, 1.0);
        return result;
    }
    if (degree == 3) {
        // Paper Eq. (3): the three undirected edges each have weight 1/2.
        add(0, 1, 0.5); add(0, 2, 0.5); add(1, 2, 0.5);
        return result;
    }
    // Paper Eqs. (6)-(8): 2p-3 active edges, built directly in O(p).
    // The paper does not specify extrema tie-breaking. Stable net-local pin
    // indices choose the first minimum and last maximum; this changes no value.
    std::size_t min_rep = 0, max_rep = 0;
    for (std::size_t i = 1; i < degree; ++i) {
        if (values[i] < values[min_rep]) min_rep = i;
        if (values[i] >= values[max_rep]) max_rep = i;
    }
    if (min_rep == max_rep) max_rep = min_rep == degree - 1 ? 0 : degree - 1;
    const double weight = 1.0 / static_cast<double>(degree - 1);
    for (std::size_t i = 0; i < degree; ++i) if (i != min_rep) add(min_rep, i, weight);
    for (std::size_t i = 0; i < degree; ++i) if (i != min_rep && i != max_rep) add(max_rep, i, weight);
    return result;
}

void extremaGradient(const Level &level, const LNet &net, const std::vector<double> &values, std::vector<double> &gradient)
{
    if (values.size() < 2) return;
    const auto [minimum, maximum] = std::minmax_element(values.begin(), values.end());
    if (tied(*minimum, *maximum)) return;
    std::set<std::size_t> low, high;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (tied(values[i], *minimum)) low.insert(net.pins[i].object_id);
        if (tied(values[i], *maximum)) high.insert(net.pins[i].object_id);
    }
    for (const auto id : low) if (!level.objects[id].fixed) gradient[id] -= 1.0 / static_cast<double>(low.size());
    for (const auto id : high) if (!level.objects[id].fixed) gradient[id] += 1.0 / static_cast<double>(high.size());
}
}

double netHpwl(const Level &level, const LNet &net)
{
    if (net.pins.size() <= 1) return 0.0;
    double xmin = std::numeric_limits<double>::infinity(), xmax = -xmin;
    double ymin = std::numeric_limits<double>::infinity(), ymax = -ymin;
    for (const auto &pin : net.pins) {
        if (pin.object_id >= level.objects.size()) throw std::runtime_error("wirelength pin object out of range in net " + net.name);
        const auto &object = level.objects[pin.object_id];
        // Paper Eq. (2): LObject x/y are lower-left, while LPin offsets are
        // relative to the object center, so the physical pin is center+offset.
        const double x = object.cx() + pin.offset_x, y = object.cy() + pin.offset_y;
        requireFinite(x, "pin x in net " + net.name); requireFinite(y, "pin y in net " + net.name);
        xmin = std::min(xmin, x); xmax = std::max(xmax, x);
        ymin = std::min(ymin, y); ymax = std::max(ymax, y);
    }
    const double value = (xmax - xmin) + (ymax - ymin);
    requireFinite(value, "HPWL in net " + net.name);
    return value;
}

double exactHpwl(const Level &level)
{
    double value = 0.0;
    for (const auto &net : level.nets) value += netHpwl(level, net);
    requireFinite(value, "total HPWL");
    return value;
}

WirelengthEvaluator::WirelengthEvaluator(std::uint64_t seed) : random_(seed) {}
void WirelengthEvaluator::reset(std::uint64_t seed) { random_.seed(seed); has_previous_ = false; previous_x_.clear(); previous_y_.clear(); }

WireEval WirelengthEvaluator::evaluate(const Level &level, WirelengthMode mode, bool update_state)
{
    const bool topology_matches = has_previous_ && level.index == level_index_ && previous_x_.size() == level.nets.size() &&
        std::equal(previous_x_.begin(), previous_x_.end(), level.nets.begin(), [](const auto &p, const auto &n){ return p.size() == n.pins.size(); });
    if (!topology_matches) { has_previous_ = false; previous_x_.clear(); previous_y_.clear(); }
    WireEval result; result.gx.assign(level.objects.size(), 0.0); result.gy.assign(level.objects.size(), 0.0);
    std::vector<std::vector<double>> current_x(level.nets.size()), current_y(level.nets.size());
    for (std::size_t nid = 0; nid < level.nets.size(); ++nid) {
        const auto &net = level.nets[nid]; const auto degree = net.pins.size();
        auto &xs = current_x[nid]; auto &ys = current_y[nid]; xs.resize(degree); ys.resize(degree);
        for (std::size_t i = 0; i < degree; ++i) {
            if (net.pins[i].object_id >= level.objects.size()) throw std::runtime_error("wirelength pin object out of range in net " + net.name);
            const auto &object = level.objects[net.pins[i].object_id]; xs[i] = object.cx() + net.pins[i].offset_x; ys[i] = object.cy() + net.pins[i].offset_y;
            requireFinite(xs[i], "pin x in net " + net.name); requireFinite(ys[i], "pin y in net " + net.name);
        }
        const double exact = netHpwl(level, net); result.hpwl += exact;
        auto previous_x = [&](std::size_t i, std::size_t j) -> std::optional<std::pair<double,double>> { if (!update_state || !has_previous_) return std::nullopt; return std::pair<double,double>{previous_x_[nid][i], previous_x_[nid][j]}; };
        auto previous_y = [&](std::size_t i, std::size_t j) -> std::optional<std::pair<double,double>> { if (!update_state || !has_previous_) return std::nullopt; return std::pair<double,double>{previous_y_[nid][i], previous_y_[nid][j]}; };
        const auto x = paperAxis(level, net, xs, result.gx, random_, previous_x);
        const auto y = paperAxis(level, net, ys, result.gy, random_, previous_y);
        const auto [xmin, xmax] = degree == 0 ? std::pair{xs.end(), xs.end()} : std::minmax_element(xs.begin(), xs.end());
        const auto [ymin, ymax] = degree == 0 ? std::pair{ys.end(), ys.end()} : std::minmax_element(ys.begin(), ys.end());
        const double exact_x = degree <= 1 ? 0.0 : *xmax - *xmin;
        const double exact_y = degree <= 1 ? 0.0 : *ymax - *ymin;
        if (std::abs(x.value - exact_x) > 1.0e-12 * std::max(1.0, std::abs(exact_x)) ||
            std::abs(y.value - exact_y) > 1.0e-12 * std::max(1.0, std::abs(exact_y)))
            throw std::runtime_error("paper L1/axis HPWL mismatch in net " + net.name);
        const double paper = x.value + y.value;
        result.paper_l1_value += paper; result.effective_edge_count_x += x.edges; result.effective_edge_count_y += y.edges; result.tie_count += x.ties + y.ties;
        const double difference = std::abs(paper - exact); result.max_l1_hpwl_difference = std::max(result.max_l1_hpwl_difference, difference);
        if (difference > 1.0e-12 * std::max(1.0, std::abs(exact))) throw std::runtime_error("paper L1/HPWL mismatch in net " + net.name);
        if (mode == WirelengthMode::Extrema) { std::fill(result.gx.begin(), result.gx.end(), 0.0); std::fill(result.gy.begin(), result.gy.end(), 0.0); }
    }
    if (mode == WirelengthMode::Extrema) for (std::size_t nid = 0; nid < level.nets.size(); ++nid) { extremaGradient(level, level.nets[nid], current_x[nid], result.gx); extremaGradient(level, level.nets[nid], current_y[nid], result.gy); }
    result.effective_edge_count = result.effective_edge_count_x + result.effective_edge_count_y;
    if (std::abs(result.paper_l1_value - result.hpwl) > 1.0e-12 * std::max(1.0, std::abs(result.hpwl)))
        throw std::runtime_error("total paper L1/HPWL mismatch");
    requireFinite(result.hpwl, "total HPWL"); requireFinite(result.paper_l1_value, "total paper L1");
    for (const double value : result.gx) requireFinite(value, "x gradient");
    for (const double value : result.gy) requireFinite(value, "y gradient");
    if (update_state) { previous_x_ = std::move(current_x); previous_y_ = std::move(current_y); level_index_ = level.index; has_previous_ = true; }
    return result;
}

WireEval evaluateWirelength(const Level &level, WirelengthMode mode) { WirelengthEvaluator evaluator(0U); return evaluator.evaluate(level, mode, false); }
WireEval wirelengthSubgradient(const Level &level, WirelengthMode mode) { return evaluateWirelength(level, mode); }

InterlevelHpwl interlevelHpwlConsistency(const Level &coarse, const Level &fine)
{
    InterlevelHpwl result; result.coarse_hpwl = exactHpwl(coarse); result.fine_hpwl = exactHpwl(fine); result.delta = result.fine_hpwl - result.coarse_hpwl;
    result.relative_delta = std::abs(result.delta) / std::max(EPS, std::abs(result.coarse_hpwl)); result.ratio = result.fine_hpwl / std::max(EPS, result.coarse_hpwl);
    result.coarse_nets = coarse.nets.size(); result.fine_nets = fine.nets.size();
    const auto index = [](const Level &level, const char *label) {
        std::map<std::size_t, const LNet *> by_id;
        for (const auto &net : level.nets) {
            if (net.original_net_id == std::numeric_limits<std::size_t>::max())
                throw std::runtime_error(std::string("invalid original_net_id in ") + label + " level " + std::to_string(level.index) + " net " + net.name);
            if (!by_id.emplace(net.original_net_id, &net).second)
                throw std::runtime_error(std::string("duplicate original_net_id in ") + label + " level " + std::to_string(level.index) + " net " + net.name + " id=" + std::to_string(net.original_net_id));
        }
        return by_id;
    };
    const auto coarse_by_id = index(coarse, "coarse");
    const auto fine_by_id = index(fine, "fine");
    for (const auto &[id, fine_net] : fine_by_id) {
        const auto found = coarse_by_id.find(id);
        if (found == coarse_by_id.end()) {
            const double contribution = netHpwl(fine, *fine_net);
            ++result.fine_only_nets; result.internalized_fine_hpwl += contribution;
            result.sum_abs_net_delta += std::abs(contribution); result.max_abs_net_delta = std::max(result.max_abs_net_delta, std::abs(contribution));
        } else {
            const double coarse_value = netHpwl(coarse, *found->second), fine_value = netHpwl(fine, *fine_net);
            ++result.paired_nets; result.matched_coarse_hpwl += coarse_value; result.matched_fine_hpwl += fine_value;
            const double difference = std::abs(coarse_value - fine_value);
            result.sum_abs_net_delta += difference; result.max_abs_net_delta = std::max(result.max_abs_net_delta, difference);
        }
    }
    for (const auto &[id, coarse_net] : coarse_by_id) if (fine_by_id.find(id) == fine_by_id.end()) {
        const double contribution = netHpwl(coarse, *coarse_net);
        ++result.coarse_only_nets; result.coarse_only_hpwl += contribution;
        result.sum_abs_net_delta += std::abs(contribution); result.max_abs_net_delta = std::max(result.max_abs_net_delta, std::abs(contribution));
    }
    return result;
}
}

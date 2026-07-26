#include "placer/multilevel/Clusterer.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <set>
#include <stdexcept>
#include <tuple>

namespace placer
{
namespace
{
constexpr std::size_t INVALID_ID = std::numeric_limits<std::size_t>::max();

struct Endpoint
{
    bool candidate_group{};
    std::size_t id{};
    bool operator<(const Endpoint &other) const { return std::tie(candidate_group, id) < std::tie(other.candidate_group, other.id); }
};

struct PairScore
{
    double score{};
    std::size_t a{}, b{};
};

struct LowerPriority
{
    bool operator()(const PairScore &lhs, const PairScore &rhs) const
    {
        if (lhs.score != rhs.score) return lhs.score < rhs.score;
        if (lhs.a != rhs.a) return lhs.a > rhs.a;
        return lhs.b > rhs.b;
    }
};

double groupArea(const Level &level, const std::vector<std::size_t> &group)
{
    double area = 0.0;
    for (const auto id : group) area += level.objects[id].area();
    if (!(area > 0.0) || !std::isfinite(area)) throw std::runtime_error("cluster group has non-positive or non-finite area");
    return area;
}

double scoreTerm(double degree, double internal, double area)
{
    if (!std::isfinite(degree) || !std::isfinite(internal) || !(internal > 0.0))
        throw std::runtime_error("cluster connectivity is non-finite or non-positive");
    // Eq. (19): a connection with no remaining external degree is completely
    // internalizable and deterministically receives the highest priority.
    const double denominator = degree - internal;
    if (denominator <= EPS) return std::numeric_limits<double>::infinity();
    const double value = internal / (denominator * area);
    if (!std::isfinite(value)) throw std::runtime_error("cluster score is non-finite");
    return value;
}

void groupConnectivity(const Level &level, const std::vector<std::size_t> &group_of,
                       std::map<std::pair<std::size_t, std::size_t>, double> &connection,
                       std::map<std::size_t, double> &external)
{
    for (const auto &net : level.nets) {
        std::set<Endpoint> endpoints;
        for (const auto &pin : net.pins) {
            if (pin.object_id >= level.objects.size()) throw std::runtime_error("cluster pin object out of range in net " + net.name);
            const auto &object = level.objects[pin.object_id];
            if (!object.fixed && !object.is_macro) {
                const auto group = group_of[pin.object_id];
                if (group == INVALID_ID) throw std::runtime_error("movable standard object missing active group in net " + net.name);
                endpoints.insert({true, group});
            } else {
                // Fixed objects and macros remain distinct hypergraph endpoints,
                // even though they are not legal standard-cell merge candidates.
                endpoints.insert({false, pin.object_id});
            }
        }
        for (const auto &endpoint : endpoints) if (endpoint.candidate_group) external[endpoint.id] += 1.0;
        if (endpoints.size() < 2) continue;
        const double weight = 1.0 / static_cast<double>(endpoints.size() - 1);
        std::vector<std::size_t> candidates;
        for (const auto &endpoint : endpoints) if (endpoint.candidate_group) candidates.push_back(endpoint.id);
        for (std::size_t i = 0; i < candidates.size(); ++i)
            for (std::size_t j = i + 1; j < candidates.size(); ++j)
                connection[std::minmax(candidates[i], candidates[j])] += weight;
    }
}

void validateHierarchyTransition(const Level &fine, const Level &coarse)
{
    if (!coarse.fine_to_coarse || coarse.fine_to_coarse->size() != fine.objects.size())
        throw std::runtime_error("hierarchy transition has invalid fine_to_coarse size");
    std::vector<unsigned> ownership(fine.objects.size(), 0U);
    for (std::size_t parent = 0; parent < coarse.objects.size(); ++parent) {
        const auto &object = coarse.objects[parent];
        if (object.children.empty()) throw std::runtime_error("coarse object " + object.name + " has no children");
        if (!(object.width > 0.0) || !(object.height > 0.0) || !std::isfinite(object.x) || !std::isfinite(object.y) ||
            !std::isfinite(object.width) || !std::isfinite(object.height)) throw std::runtime_error("non-finite coarse geometry for " + object.name);
        for (const auto child : object.children) {
            if (child >= fine.objects.size() || ++ownership[child] != 1U || (*coarse.fine_to_coarse)[child] != parent)
                throw std::runtime_error("invalid or duplicate hierarchy child in " + object.name);
            if (fine.objects[child].fixed && !object.fixed) throw std::runtime_error("fixed child became movable in " + object.name);
            if (fine.objects[child].is_macro && object.children.size() != 1) throw std::runtime_error("macro merged into cluster " + object.name);
        }
    }
    for (std::size_t child = 0; child < ownership.size(); ++child)
        if (ownership[child] != 1U || (*coarse.fine_to_coarse)[child] >= coarse.objects.size()) throw std::runtime_error("fine object has no valid coarse owner");
    std::set<std::size_t> net_ids;
    for (const auto &net : coarse.nets) {
        if (net.original_net_id == INVALID_ID) throw std::runtime_error("invalid original_net_id in coarse net " + net.name);
        if (!net_ids.insert(net.original_net_id).second) throw std::runtime_error("duplicate original_net_id in coarse net " + net.name);
        for (const auto &pin : net.pins) if (pin.object_id >= coarse.objects.size()) throw std::runtime_error("coarse pin object out of range in net " + net.name);
    }
}
}

Level clusterOneLevel(const Level &fine, const ClusterConfig &config)
{
    const auto target = static_cast<std::size_t>(std::max(1.0, std::ceil(static_cast<double>(fine.movableIds().size()) / config.coarsen_ratio)));
    return clusterOneLevel(fine, target, config.degree_cap);
}

Level clusterOneLevel(const Level &fine, std::size_t target_movable, int degree_cap)
{
    const auto standard_ids = fine.standardMovableIds();
    const auto macro_ids = fine.macroIds();
    const std::size_t desired_standard = std::max<std::size_t>(1, target_movable > macro_ids.size() ? target_movable - macro_ids.size() : 1);
    std::vector<std::vector<std::size_t>> groups;
    for (const auto id : standard_ids) groups.push_back({id});
    (void)degree_cap; // Compatibility option; the paper path does not cap degree.

    while (groups.size() > desired_standard) {
        std::vector<std::size_t> group_of(fine.objects.size(), INVALID_ID);
        for (std::size_t group = 0; group < groups.size(); ++group) for (const auto id : groups[group]) group_of[id] = group;
        std::map<std::pair<std::size_t, std::size_t>, double> connection;
        std::map<std::size_t, double> external;
        groupConnectivity(fine, group_of, connection, external);
        std::priority_queue<PairScore, std::vector<PairScore>, LowerPriority> queue;
        for (const auto &[pair, internal] : connection) {
            const auto [a, b] = pair;
            const double first = scoreTerm(external.at(a), internal, groupArea(fine, groups[a]));
            const double second = scoreTerm(external.at(b), internal, groupArea(fine, groups[b]));
            const double score = std::isinf(first) || std::isinf(second) ? std::numeric_limits<double>::infinity() : first + second;
            if (score > 0.0) queue.push({score, a, b});
        }
        if (queue.empty()) break;
        const auto best = queue.top();
        groups[best.a].insert(groups[best.a].end(), groups[best.b].begin(), groups[best.b].end());
        groups.erase(groups.begin() + static_cast<std::ptrdiff_t>(best.b));
    }

    Level coarse;
    coarse.index = fine.index + 1;
    coarse.fine_to_coarse = std::vector<std::size_t>(fine.objects.size(), INVALID_ID);
    auto makeObject = [&](const std::vector<std::size_t> &children, const std::string &name, bool macro, bool fixed) {
        LObject object;
        object.name = name; object.fixed = fixed; object.is_macro = macro; object.children = children;
        double area = 0.0, weighted_x = 0.0, weighted_y = 0.0, weighted_aspect = 0.0;
        for (const auto child : children) {
            const auto &fine_object = fine.objects[child];
            const double child_area = fine_object.area();
            if (!(child_area > 0.0) || !std::isfinite(child_area)) throw std::runtime_error("invalid child area for " + fine_object.name);
            area += child_area; weighted_x += child_area * fine_object.cx(); weighted_y += child_area * fine_object.cy();
            weighted_aspect += child_area * (fine_object.width / fine_object.height);
            object.members.insert(object.members.end(), fine_object.members.begin(), fine_object.members.end());
            object.child_offsets[child] = {0.0, 0.0};
        }
        if (fixed || (macro && children.size() == 1)) {
            const auto &source = fine.objects[children.front()];
            object.width = source.width; object.height = source.height; object.x = source.x; object.y = source.y;
        } else {
            // Paper mode represents physical area, not an artificial packed child layout.
            const double aspect = std::clamp(weighted_aspect / area, 0.25, 4.0);
            object.width = std::sqrt(area * aspect); object.height = area / object.width;
            object.setCenter(weighted_x / area, weighted_y / area);
        }
        const auto parent = coarse.objects.size();
        for (const auto child : children) (*coarse.fine_to_coarse)[child] = parent;
        coarse.objects.push_back(std::move(object));
    };
    for (std::size_t i = 0; i < groups.size(); ++i) makeObject(groups[i], "cluster_L" + std::to_string(coarse.index) + "_" + std::to_string(i), false, false);
    for (const auto id : macro_ids) makeObject({id}, fine.objects[id].name, true, false);
    for (std::size_t id = 0; id < fine.objects.size(); ++id) if (fine.objects[id].fixed) makeObject({id}, fine.objects[id].name, fine.objects[id].is_macro, true);

    std::set<std::size_t> original_ids;
    for (const auto &net : fine.nets) {
        if (net.original_net_id == INVALID_ID) throw std::runtime_error("invalid original_net_id in fine net " + net.name);
        if (!original_ids.insert(net.original_net_id).second) throw std::runtime_error("duplicate original_net_id in fine net " + net.name + " id=" + std::to_string(net.original_net_id));
        LNet coarse_net;
        coarse_net.name = net.name;
        coarse_net.original_net_id = net.original_net_id;
        coarse_net.original_net_name = net.original_net_name;
        std::set<std::size_t> endpoints;
        for (const auto &pin : net.pins) {
            if (pin.object_id >= fine.objects.size()) throw std::runtime_error("fine pin object out of range in net " + net.name);
            const auto parent = (*coarse.fine_to_coarse)[pin.object_id];
            if (parent >= coarse.objects.size()) throw std::runtime_error("invalid fine_to_coarse mapping in net " + net.name);
            endpoints.insert(parent);
        }
        for (const auto parent : endpoints) coarse_net.pins.push_back({parent, 0.0, 0.0});
        if (coarse_net.pins.size() >= 2) coarse.nets.push_back(std::move(coarse_net));
    }
    std::sort(coarse.nets.begin(), coarse.nets.end(), [](const LNet &a, const LNet &b) { return a.original_net_id < b.original_net_id; });
    validateHierarchyTransition(fine, coarse);
    return coarse;
}

std::vector<Level> buildHierarchy(const Level &level0, const ClusterConfig &config)
{
    std::vector<Level> hierarchy{level0};
    const auto threshold = static_cast<std::size_t>(config.current) * static_cast<std::size_t>(config.current);
    while (hierarchy.back().movableIds().size() > threshold && static_cast<int>(hierarchy.size()) < config.max_levels) {
        const auto &fine = hierarchy.back();
        const auto movable = fine.movableIds().size();
        const auto target = std::max(threshold, static_cast<std::size_t>(std::ceil(static_cast<double>(movable) / config.coarsen_ratio)));
        const auto start = std::chrono::steady_clock::now();
        std::cout << "[cluster L" << fine.index << "] movable=" << movable << " target=" << target << '\n';
        auto coarse = clusterOneLevel(fine, target, config.degree_cap);
        const auto coarse_movable = coarse.movableIds().size();
        const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        std::cout << "[cluster L" << fine.index << "] result movable=" << coarse_movable << " objects=" << coarse.objects.size() << " nets=" << coarse.nets.size() << " elapsed=" << elapsed << '\n';
        if (coarse_movable >= movable) { std::cout << "[cluster] no reduction; stopping hierarchy\n"; break; }
        hierarchy.push_back(std::move(coarse));
    }
    return hierarchy;
}
}

#include "placer/multilevel/Declusterer.hpp"
#include <stdexcept>
namespace placer
{
    DeclusterStats decluster(Level &fine, Level &coarse, const Region &region)
    {
        (void)region; // Projection is deliberately owned by the caller.
        if (!coarse.fine_to_coarse || coarse.fine_to_coarse->size() != fine.objects.size())
            throw std::runtime_error("invalid fine_to_coarse size for decluster from level " + std::to_string(coarse.index));
        DeclusterStats stats;
        stats.parents = coarse.objects.size();
        for (std::size_t child = 0; child < fine.objects.size(); ++child) {
            const auto parent = (*coarse.fine_to_coarse)[child];
            if (parent >= coarse.objects.size()) throw std::runtime_error("invalid parent id for fine child " + std::to_string(child));
            if (fine.objects[child].fixed) { ++stats.fixed_children_unchanged; continue; }
            // Fig. 3: every movable child inherits the optimized parent center.
            // child_offsets and region constraints play no role at this stage.
            fine.objects[child].setCenter(coarse.objects[parent].cx(), coarse.objects[parent].cy());
            ++stats.inherited_children;
        }
        // Compatibility fields remain zero: declustering never shifts a parent
        // and no packed-offset feasibility test is performed.
        return stats;
    }
}

#include "../TestSupport.hpp"
#include "placer/multilevel/Clusterer.hpp"
#include "placer/database/PlacementDB.hpp"
#include "placer/io/BookshelfReader.hpp"
#include "placer/objective/Wirelength.hpp"
#include <algorithm>
#include <limits>

namespace {
placer::LObject object(const char *name, double x, bool fixed=false, bool macro=false)
{
    placer::LObject value; value.name=name; value.width=1.0; value.height=1.0; value.x=x; value.fixed=fixed; value.is_macro=macro; return value;
}
placer::LNet net(const char *name, std::size_t id, std::initializer_list<std::size_t> pins)
{
    placer::LNet value; value.name=name; value.original_net_name=name; value.original_net_id=id;
    for (auto pin:pins) value.pins.push_back({pin,0.25,-0.25});
    return value;
}
}

int main()
{
    // ABF has d=3 and D_AB=1/2. AC has d=2 and D_AC=1, so corrected
    // Eq. (19) selects AC. Omitting F would make AB tie at infinity and win by ID.
    placer::Level fixed; fixed.objects={object("A",0),object("B",2),object("C",4),object("F",6,true)};
    fixed.nets={net("ABF",10,{0,1,3}),net("AC",11,{0,2})};
    auto coarse=placer::clusterOneLevel(fixed,2,256);
    CHECK_EQ(coarse.objects.size(),3u);
    CHECK_EQ((*coarse.fine_to_coarse)[0],(*coarse.fine_to_coarse)[2]);
    CHECK((*coarse.fine_to_coarse)[0]!=(*coarse.fine_to_coarse)[1]);
    CHECK(coarse.objects[(*coarse.fine_to_coarse)[3]].fixed);

    placer::Level macro; macro.objects={object("A",0),object("B",2),object("M",4,false,true)};
    macro.nets={net("ABM",20,{0,1,2})};
    auto macro_coarse=placer::clusterOneLevel(macro,2,256);
    CHECK_EQ(macro_coarse.objects.size(),2u);
    CHECK_EQ((*macro_coarse.fine_to_coarse)[0],(*macro_coarse.fine_to_coarse)[1]);
    CHECK((*macro_coarse.fine_to_coarse)[2]!=(*macro_coarse.fine_to_coarse)[0]);
    CHECK(macro_coarse.objects[(*macro_coarse.fine_to_coarse)[2]].is_macro);

    // Duplicate fine pins become one zero-offset pin per coarse object, with
    // stable object ordering and preserved identity. The AB net internalizes.
    placer::Level pins; pins.objects={object("A",0),object("B",2),object("C",8)};
    pins.nets={net("ABC",31,{1,2,0}),net("AB",30,{0,1})};
    auto pin_coarse=placer::clusterOneLevel(pins,2,256);
    CHECK_EQ(pin_coarse.nets.size(),1u);
    const auto &cn=pin_coarse.nets.front();
    CHECK_EQ(cn.original_net_id,31u); CHECK_EQ(cn.original_net_name,"ABC"); CHECK_EQ(cn.pins.size(),2u);
    CHECK(cn.pins[0].object_id<cn.pins[1].object_id);
    for(const auto &pin:cn.pins){ CHECK_EQ(pin.offset_x,0.0); CHECK_EQ(pin.offset_y,0.0); }
    for(const auto &obj:pin_coarse.objects) for(const auto &[child,offset]:obj.child_offsets) { (void)child; CHECK_EQ(offset.first,0.0); CHECK_EQ(offset.second,0.0); }
    const auto consistency=placer::interlevelHpwlConsistency(pin_coarse,pins);
    CHECK_EQ(consistency.fine_only_nets,1u); CHECK_EQ(consistency.coarse_only_nets,0u);
    CHECK_NEAR(consistency.internalized_fine_hpwl,2.0,1e-12);
    // Tiny fixture: force a two-level hierarchy and verify deterministic mapping,
    // legal pins, finite geometry, and parent-preserving declustering separately.
    const auto db=placer::loadBookshelf("tests/data/tiny/tiny_basic/tiny_basic.aux");
    const auto level0=placer::buildLevel0(db);
    const placer::ClusterConfig config{1,2.0,2,256};
    auto hierarchy=placer::buildHierarchy(level0,config);
    auto repeated=placer::buildHierarchy(level0,config);
    CHECK_EQ(hierarchy.size(),2u); CHECK_EQ(repeated.size(),hierarchy.size());
    CHECK(*hierarchy[1].fine_to_coarse==*repeated[1].fine_to_coarse);
    CHECK_EQ(hierarchy[1].nets.size(),repeated[1].nets.size());
    for(const auto &level:hierarchy){
        for(const auto &obj:level.objects) CHECK(std::isfinite(obj.x)&&std::isfinite(obj.y)&&std::isfinite(obj.width)&&std::isfinite(obj.height));
        for(const auto &n:level.nets) for(const auto &p:n.pins) CHECK(p.object_id<level.objects.size());
    }
    return 0;
}

#include "../TestSupport.hpp"
#include "placer/objective/Wirelength.hpp"
#include <algorithm>
#include <limits>
namespace {
placer::LNet net(const char*n,std::size_t id,std::size_t a,std::size_t b){placer::LNet x;x.name=n;x.original_net_name=n;x.original_net_id=id;x.pins={{a,0,0},{b,0,0}};return x;}
placer::Level level(){placer::Level l;l.objects.resize(4);for(std::size_t i=0;i<4;++i){l.objects[i].width=1;l.objects[i].height=1;l.objects[i].x=static_cast<double>(i);}return l;}
}
int main()
{
 auto coarse=level(),fine=level(); coarse.index=1;
 // Coarse order is 2,1,4; fine order is 3,1,2. ID pairing gives deltas:
 // id1 |1-2|=1, id2 |2-3|=1, fine-only id3=3, coarse-only id4=1.
 coarse.nets={net("c2",2,0,2),net("c1",1,0,1),net("c4",4,0,1)};
 fine.nets={net("f3",3,0,3),net("f1",1,0,2),net("f2",2,0,3)};
 auto r=placer::interlevelHpwlConsistency(coarse,fine);
 CHECK_EQ(r.paired_nets,2u);CHECK_EQ(r.fine_only_nets,1u);CHECK_EQ(r.coarse_only_nets,1u);
 CHECK_NEAR(r.sum_abs_net_delta,6.0,1e-12);CHECK_NEAR(r.max_abs_net_delta,3.0,1e-12);
 CHECK_NEAR(r.matched_coarse_hpwl,3.0,1e-12);CHECK_NEAR(r.matched_fine_hpwl,5.0,1e-12);
 CHECK_NEAR(r.internalized_fine_hpwl,3.0,1e-12);CHECK_NEAR(r.coarse_only_hpwl,1.0,1e-12);
 std::reverse(coarse.nets.begin(),coarse.nets.end()); auto reordered=placer::interlevelHpwlConsistency(coarse,fine);
 CHECK_EQ(reordered.sum_abs_net_delta,r.sum_abs_net_delta);
 auto duplicate=fine;duplicate.nets.push_back(net("duplicate",1,0,1));CHECK_THROW(placer::interlevelHpwlConsistency(coarse,duplicate));
 auto invalid=fine;invalid.nets[0].original_net_id=std::numeric_limits<std::size_t>::max();CHECK_THROW(placer::interlevelHpwlConsistency(coarse,invalid));
 return 0;
}

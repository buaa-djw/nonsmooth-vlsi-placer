#include "placer/io/BookshelfReader.hpp"
#include "placer/objective/Wirelength.hpp"
#include "../TestSupport.hpp"
int main(){
 auto db=placer::loadBookshelf("tests/data/tiny/tiny_basic/tiny_basic.aux"); auto l=placer::buildLevel0(db);
 auto w=placer::wirelengthSubgradient(l,placer::WirelengthMode::Extrema); CHECK_NEAR(w.hpwl,placer::exactHpwl(l),1e-12);
 placer::Level p; for(int i=0;i<5;++i)p.objects.push_back({std::to_string(i),1,1,double(i),double((i*3)%5),i==4,false});
 placer::LNet net{"five",{}}; for(size_t i=0;i<5;++i)net.pins.push_back({i,0.1*double(i),-0.2*double(i)});p.nets.push_back(net);
 auto paper=placer::wirelengthSubgradient(p,placer::WirelengthMode::PaperL1);
 CHECK_NEAR(paper.hpwl,placer::exactHpwl(p),1e-12);
 CHECK_NEAR(paper.gx[4],0.0,0.0); CHECK_NEAR(paper.gy[4],0.0,0.0);
 // The two x-inner pins have no mutual Equation (6) contribution; the full
 // pair model nevertheless has the same value as independent HPWL.
 CHECK_EQ(paper.gx.size(),p.objects.size());
 return 0;
}

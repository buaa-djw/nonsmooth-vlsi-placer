#include "placer/io/BookshelfReader.hpp"
#include "placer/objective/Density.hpp"
#include "../TestSupport.hpp"
int main(){
    auto db=placer::loadBookshelf("tests/data/tiny/tiny_basic/tiny_basic.aux");
    auto l=placer::buildLevel0(db); placer::DensityGrid dg(db.region(),4,5,1.0,1.0);
    auto e=dg.evaluate(l); CHECK_EQ(e.gx.size(),l.objects.size()); CHECK(e.penalty>=0.0);

    placer::Level manual;
    manual.objects.push_back({"fixed",8,8,0,0,true,false});
    manual.objects.push_back({"movable",2,2,8,8,false,false});
    placer::DensityGrid one({0,10,0,10},1,1,0.5,0.5);
    auto m=one.evaluate(manual);
    CHECK_NEAR(m.total_object_area,68.0,1e-12);
    CHECK_NEAR(m.total_fixed_area,64.0,1e-12);
    CHECK_NEAR(m.total_movable_area,4.0,1e-12);
    CHECK_NEAR(m.total_overflow,18.0,1e-12);
    CHECK_NEAR(m.quadratic_penalty,324.0,1e-12);
    CHECK_NEAR(m.paper_ofr,18.0/68.0,1e-12);
    CHECK_EQ(m.overflow_bin_count,1u);
    CHECK_NEAR(m.max_bin_overflow,18.0,1e-12);
    CHECK_NEAR(m.gx[0],0.0,0.0); CHECK_NEAR(m.gy[0],0.0,0.0);

    placer::Level outside;
    outside.objects.push_back({"partly-outside-fixed",10,10,-5,0,true,false});
    auto o=one.evaluate(outside);
    CHECK_NEAR(o.total_object_area,100.0,1e-12);
    CHECK_NEAR(o.total_clipped_area,50.0,1e-12);
    CHECK_NEAR(o.paper_ofr,0.0,1e-12);
    placer::Level empty; CHECK_THROW(one.evaluate(empty));
    return 0;
}

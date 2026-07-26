#include "../TestSupport.hpp"
#include "placer/multilevel/Declusterer.hpp"
int main()
{
    placer::Level fine; fine.objects.resize(3);
    for(auto &o:fine.objects){o.width=2;o.height=2;}
    fine.objects[0].x=1; fine.objects[1].x=5; fine.objects[2].x=9; fine.objects[2].fixed=true;
    placer::Level coarse; coarse.index=1; coarse.objects.resize(1); coarse.objects[0].width=4; coarse.objects[0].height=4; coarse.objects[0].setCenter(20,30);
    coarse.objects[0].child_offsets={{0,{100,200}},{1,{-100,-200}},{2,{3,4}}}; coarse.fine_to_coarse=std::vector<std::size_t>{0,0,0};
    const double px=coarse.objects[0].x,py=coarse.objects[0].y,fx=fine.objects[2].x,fy=fine.objects[2].y;
    const auto stats=placer::decluster(fine,coarse,{0,10,0,10});
    CHECK_EQ(fine.objects[0].cx(),20.0); CHECK_EQ(fine.objects[0].cy(),30.0);
    CHECK_EQ(fine.objects[1].cx(),20.0); CHECK_EQ(fine.objects[1].cy(),30.0);
    CHECK_EQ(fine.objects[2].x,fx); CHECK_EQ(fine.objects[2].y,fy);
    CHECK_EQ(coarse.objects[0].x,px); CHECK_EQ(coarse.objects[0].y,py);
    CHECK_EQ(stats.inherited_children,2u); CHECK_EQ(stats.fixed_children_unchanged,1u);
    CHECK_EQ(stats.shifted_parents,0u); CHECK_EQ(stats.impossible_groups,0u); CHECK_EQ(stats.max_parent_shift,0.0);
    return 0;
}

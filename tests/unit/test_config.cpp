#include "placer/Config.hpp"
#include <cmath>
#include <iostream>
#define CHECK(x) do{if(!(x)){std::cerr<<"CHECK failed: " #x "\n"; return 1;}}while(0)
int main(){ const char* a1[]={"p","a.aux"}; auto c=placer::parseConfig(2,const_cast<char**>(a1)); CHECK(c.current==150); CHECK(c.quadratic_init); CHECK(c.wirelength_mode==placer::WirelengthMode::PaperL1); const char* a2[]={"p","a.aux","--out","o","--wirelength-mode","extrema","--target-density","0.5","--bins","4","5","--iterations","7","--no-macro-shifting"}; auto d=placer::parseConfig(14,const_cast<char**>(a2)); CHECK(d.out=="o"); CHECK(d.wirelength_mode==placer::WirelengthMode::Extrema); CHECK(std::abs(d.target_density-0.5)<1e-12); CHECK(d.bins_x==4&&d.bins_y==5); CHECK(d.iterations_per_stage==7); CHECK(!d.macro_shifting); bool threw=false; try{const char* bad[]={"p","a.aux","--target-density","2"}; (void)placer::parseConfig(4,const_cast<char**>(bad));}catch(...){threw=true;} CHECK(threw); return 0; }

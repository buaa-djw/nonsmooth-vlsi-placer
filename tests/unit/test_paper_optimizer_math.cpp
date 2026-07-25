#include "../TestSupport.hpp"
#include "placer/optimizer/NonsmoothOptimizer.hpp"
#include <cmath>
int main(){
 CHECK_NEAR(placer::paperInitialLambda(10,2),5,1e-12);
 bool threw=false;try{(void)placer::paperInitialLambda(10,0);}catch(...){threw=true;}CHECK(threw);
 CHECK_NEAR(placer::paperStepScale(0),.2,1e-15);CHECK_NEAR(placer::paperStepScale(99),.2,1e-15);
 CHECK_NEAR(placer::paperStepScale(100),.2*2/3,1e-15);CHECK_NEAR(placer::paperStepScale(200),.2*4/9,1e-15);
 CHECK_NEAR(placer::paperStepScale(300),.06,1e-15);CHECK_NEAR(placer::paperStepScale(19999),.06,1e-15);
 std::vector<double> previous{1,2}, current{-1,1};
 CHECK_NEAR(placer::polakRibiereBeta(current,previous),0.2,1e-15); // negative components are not PR+ clipped
 CHECK_NEAR(placer::nextPaperLambda(10,.03,0,false),16,1e-12);
 CHECK_NEAR(placer::nextPaperLambda(10,.1,.3,true),19,1e-12);
 CHECK_NEAR(placer::nextPaperLambda(10,.2,.3,true),22,1e-12);
 CHECK_EQ(placer::paperNoImprovementLimit(1),1);CHECK_EQ(placer::paperNoImprovementLimit(210904),100);
 return 0;
}

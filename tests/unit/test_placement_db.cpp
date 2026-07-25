#include "placer/database/PlacementDB.hpp"
#include "../TestSupport.hpp"
int main(){ placer::PlacementDB db; auto c=db.addCell("a",2.0,3.0,false); auto n=db.addNet("n"); auto p=db.addPin(c,n,0.5,-0.5,"I"); CHECK_EQ(c,0u); CHECK_EQ(n,0u); CHECK_EQ(p,0u); CHECK_NEAR(db.cells[c].area(),6.0,1e-12); return 0; }

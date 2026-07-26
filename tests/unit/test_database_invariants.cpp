#include "placer/io/BookshelfReader.hpp"
#include "../TestSupport.hpp"
#include <cmath>
int main(){auto db=placer::loadBookshelf("tests/data/stage1/valid/valid.aux");db.validate();for(std::size_t i=0;i<db.cells.size();++i){const auto&c=db.cells[i];CHECK(!c.terminal||c.fixed);CHECK_EQ(db.cell_name_to_id.at(c.name),i);}for(std::size_t n=0;n<db.nets.size();++n)for(auto p:db.nets[n].pin_ids){CHECK(p<db.pins.size());CHECK_EQ(db.pins[p].net_id,n);CHECK(db.pins[p].cell_id<db.cells.size());}for(const auto&r:db.rows)CHECK_NEAR(r.x_end,r.x_start+r.num_sites*r.site_spacing,1e-12);CHECK_EQ(db.nets[1].pin_ids.size(),1u);return 0;}

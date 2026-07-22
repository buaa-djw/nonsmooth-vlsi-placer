#include "placer/io/BookshelfWriter.hpp"
#include <fstream>
#include <iomanip>
namespace placer { void writeLevelPl(const std::string&p,const Level&l){std::ofstream f(p); f<<"UCLA pl 1.0\n"<<std::fixed<<std::setprecision(6); for(auto&o:l.objects) f<<o.name<<'\t'<<o.x<<'\t'<<o.y<<"\t: N"<<(o.fixed?" /FIXED":"")<<'\n';} void writeFinalPl(const std::string&p,PlacementDB&db,const Level&l){for(auto&o:l.objects)if(o.members.size()==1&&!db.cells[o.members[0]].fixed){db.cells[o.members[0]].x=o.x;db.cells[o.members[0]].y=o.y;} std::ofstream f(p); f<<"UCLA pl 1.0\n"<<std::fixed<<std::setprecision(6); for(auto&c:db.cells)f<<c.name<<'\t'<<c.x<<'\t'<<c.y<<"\t: "<<c.orientation<<(c.fixed?" /FIXED":"")<<'\n';} }

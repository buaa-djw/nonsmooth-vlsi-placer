#include "placer/Config.hpp"
#include "placer/io/BookshelfReader.hpp"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <cstdlib>
static std::string quote(const std::string&s){std::string r="'"; for(char c:s){ if(c=='\'') r += "'\\''"; else r.push_back(c);} r.push_back('\''); return r;}
int main(int argc,char**argv){try{auto cfg=placer::parseConfig(argc,argv); (void)placer::loadBookshelf(cfg.aux); std::ostringstream cmd; cmd<<"python3 "<<quote("reference/python/nonsmooth_vlsi_placer_paper4_ms_no_wsa_hpwl_fixed.py"); for(auto&a:cfg.passthrough) cmd<<' '<<quote(a); int rc=std::system(cmd.str().c_str()); if(rc!=0) return 2; return 0;}catch(const std::exception&e){std::cerr<<"error: "<<e.what()<<"\n"; return 2;}}

#include "placer/Config.hpp"
#include <stdexcept>
namespace placer { Config parseConfig(int argc,char**argv){Config c; for(int i=1;i<argc;++i){std::string a=argv[i]; if(a=="--out"&&i+1<argc){c.out=argv[++i]; c.passthrough.push_back(a); c.passthrough.push_back(c.out);} else { if(a.rfind("--",0)!=0 && c.aux.empty()) c.aux=a; c.passthrough.push_back(a);} } if(c.aux.empty()) throw std::runtime_error("missing aux"); return c;} }

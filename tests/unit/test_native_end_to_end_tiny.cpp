#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#define CHECK(x) do{if(!(x)){std::cerr<<"CHECK failed: " #x "\n"; return 1;}}while(0)
int main(){std::filesystem::remove_all("output/test_native_e2e"); int rc=std::system("./build/nonsmooth_placer tests/data/tiny/tiny_basic/tiny_basic.aux --iterations-per-stage 2 --penalty-stages 2 --quadratic-iterations 2 --out output/test_native_e2e"); CHECK(rc==0); for(auto f:{"hierarchy.json","coarsest_before_quadratic.pl","coarsest_after_quadratic.pl","history.csv","interlevel_hpwl.json","summary.json","run_info.json","level_0_final.pl","final.pl"}) CHECK(std::filesystem::exists(std::filesystem::path("output/test_native_e2e")/f)); std::ifstream h("output/test_native_e2e/history.csv"); std::string line; std::getline(h,line); CHECK(line.find("raw_objective")!=std::string::npos); CHECK(line.find("total_norm")==std::string::npos); return 0;}

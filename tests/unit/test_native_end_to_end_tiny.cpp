#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#define CHECK(x) do{if(!(x)){std::cerr<<"CHECK failed: " #x "\n"; return 1;}}while(0)
int main(){std::filesystem::remove_all("output/test_native_e2e"); int rc=std::system("./build/nonsmooth_placer tests/data/tiny/tiny_basic/tiny_basic.aux --iterations-per-stage 2 --penalty-stages 1 --quadratic-iterations 2 --out output/test_native_e2e"); CHECK(rc==0); for(auto f:{"hierarchy.json","coarsest_before_quadratic.pl","coarsest_after_quadratic.pl","history.csv","interlevel_hpwl.json","summary.json","run_info.json","level_0_final.pl","final.pl"}) CHECK(std::filesystem::exists(std::filesystem::path("output/test_native_e2e")/f)); std::ifstream h("output/test_native_e2e/history.csv"); std::string line; std::getline(h,line); CHECK(line=="global_iteration,level,stage,iteration,hpwl,density_penalty,ofr_penalty,ofr_report,max_density,overflow_bins_penalty,overflow_bins_report,lambda,beta_pr,step,gradient_rms,total_norm,elapsed_sec"); return 0;}

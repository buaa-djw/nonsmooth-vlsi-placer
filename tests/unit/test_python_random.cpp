#include "placer/util/PythonRandom.hpp"
#include "../TestSupport.hpp"
#include <vector>
int main()
{
    const std::vector<std::vector<int>> expected{
        {10,18,16,14,0,17,11,2,3,9,5,7,4,19,6,15,8,1,13,12},
        {11,5,17,19,9,0,16,1,15,6,10,13,14,12,7,3,8,2,18,4},
        {7,6,17,8,19,15,13,0,3,9,14,4,10,12,16,5,11,18,2,1},
        {19,5,14,4,9,13,15,18,6,12,17,10,1,11,2,16,7,8,0,3},
        {17,6,19,8,10,11,13,12,15,7,3,1,14,4,16,2,18,5,0,9}};
    const std::vector<std::uint64_t> seeds{0,1,2,42,123456};
    for (std::size_t s=0; s<seeds.size(); ++s) {
        std::vector<int> values(20); for (int i=0;i<20;++i) values[static_cast<std::size_t>(i)] = i;
        placer::PythonRandom rng(seeds[s]); rng.shuffle(values);
        CHECK_EQ(values.size(), expected[s].size());
        for (std::size_t i=0;i<values.size();++i) CHECK_EQ(values[i], expected[s][i]);
    }
    placer::PythonRandom rng(42); bool threw=false; try { (void)rng.randBelow(0); } catch (...) { threw=true; } CHECK(threw);
    return 0;
}

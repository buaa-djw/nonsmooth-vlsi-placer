#include "placer/postprocess/WhitespaceAllocator.hpp"
#include <cmath>
#include <stdexcept>
int main()
{
    placer::Level level;
    level.index = 0;
    level.objects.push_back({"a", 1.0, 1.0, 0.0, 0.0, false, false, {0}});
    level.objects.push_back({"b", 1.0, 1.0, 8.0, 0.0, false, false, {1}});
    level.objects.push_back({"fixed", 2.0, 2.0, 4.0, 4.0, true, false, {2}});
    auto result = placer::allocateWhitespace(level, {0.0, 10.0, 0.0, 10.0}, 0.5, 1, 0.05);
    if (result.objects != 2 || result.leaves != 2) throw std::runtime_error("unexpected whitespace tree statistics");
    if (!(result.rms_displacement > 0.0)) throw std::runtime_error("expected affine allocation displacement");
    if (level.objects[0].cx() < -1.0e-12 || level.objects[1].cx() > 10.0 + 1.0e-12) throw std::runtime_error("objects outside region after allocation");
    bool threw = false;
    try { (void)placer::allocateWhitespace(level, {0.0, 10.0, 0.0, 10.0}, 0.5, 1, 0.75); }
    catch (const std::runtime_error &) { threw = true; }
    if (!threw) throw std::runtime_error("invalid minimum fraction was accepted");
    return 0;
}

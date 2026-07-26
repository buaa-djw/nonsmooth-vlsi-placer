#pragma once
#include "placer/objective/Density.hpp"
#include <string>
inline placer::Level densityLevel(const std::vector<placer::LObject>&objects){placer::Level l;l.objects=objects;return l;}

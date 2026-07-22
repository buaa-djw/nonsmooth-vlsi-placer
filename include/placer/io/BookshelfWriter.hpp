#pragma once
#include "placer/multilevel/Level.hpp"
#include "placer/database/PlacementDB.hpp"
#include <string>
namespace placer { void writeLevelPl(const std::string&, const Level&); void writeFinalPl(const std::string&, PlacementDB&, const Level&); }

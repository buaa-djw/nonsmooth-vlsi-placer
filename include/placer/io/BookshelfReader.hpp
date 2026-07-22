#pragma once
#include "placer/database/PlacementDB.hpp"
#include <map>
namespace placer { [[nodiscard]] std::map<std::string,std::string> parseAux(const std::string& aux); void parseNodes(const std::string&,PlacementDB&); void parsePl(const std::string&,PlacementDB&); void parseScl(const std::string&,PlacementDB&); void parseNets(const std::string&,PlacementDB&); [[nodiscard]] PlacementDB loadBookshelf(const std::string& aux); }

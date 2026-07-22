#pragma once
#include <string>
#include <vector>
namespace placer { [[nodiscard]] std::string readTextFile(const std::string& path); [[nodiscard]] std::vector<std::string> tokens(const std::string& line); }

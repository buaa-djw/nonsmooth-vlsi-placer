#pragma once
#include <string>
#include <vector>
namespace placer { struct Config{std::string aux; std::string out{"output/nonsmooth_single"}; std::vector<std::string> passthrough;}; [[nodiscard]] Config parseConfig(int argc,char**argv); }

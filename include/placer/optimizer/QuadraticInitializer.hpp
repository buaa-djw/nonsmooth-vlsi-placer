#pragma once
#include "placer/multilevel/Level.hpp"
namespace placer { struct QuadraticConfig{int iterations{200}; double damping{0.75}; double anchor{1e-4}; double tolerance{1e-3}; int seed{1};}; struct QuadraticResult{int iterations{}; double rms{};}; [[nodiscard]] QuadraticResult quadraticInitialize(Level&, const Region&, const QuadraticConfig&); }

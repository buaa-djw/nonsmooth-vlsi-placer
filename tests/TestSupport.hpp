#pragma once
#include <cmath>
#include <cstdlib>
#include <iostream>
#define CHECK(condition) do { if (!(condition)) { std::cerr << "CHECK failed: " #condition << " at " << __FILE__ << ':' << __LINE__ << '\n'; return 1; } } while (false)
#define CHECK_EQ(actual, expected) do { auto a_ = (actual); auto e_ = (expected); if (!(a_ == e_)) { std::cerr << "CHECK_EQ failed: " #actual " == " #expected << " at " << __FILE__ << ':' << __LINE__ << " actual=" << a_ << " expected=" << e_ << '\n'; return 1; } } while (false)
#define CHECK_NEAR(actual, expected, tolerance) do { auto a_ = (actual); auto e_ = (expected); auto t_ = (tolerance); if (std::fabs(a_ - e_) > t_) { std::cerr << "CHECK_NEAR failed: " #actual " ~= " #expected << " at " << __FILE__ << ':' << __LINE__ << " actual=" << a_ << " expected=" << e_ << " tolerance=" << t_ << '\n'; return 1; } } while (false)
#define CHECK_THROW(expression) do { bool threw_ = false; try { (void)(expression); } catch (...) { threw_ = true; } CHECK(threw_); } while (false)

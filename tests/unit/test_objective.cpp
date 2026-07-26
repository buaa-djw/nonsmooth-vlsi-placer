// Consolidated from test_objective_call_count.cpp
#include "test_objective_support.hpp"
int consolidated_objective_0(){testCalls();return 0;}

// Consolidated from test_objective_combined.cpp
#include "test_objective_support.hpp"
int consolidated_objective_1(){testCombined();return 0;}

// Consolidated from test_objective_controlled_descent.cpp
#include "test_objective_support.hpp"
int consolidated_objective_2(){testDescent();return 0;}

// Consolidated from test_objective_density_only.cpp
#include "test_objective_support.hpp"
int consolidated_objective_3(){testDensityOnly();return 0;}

// Consolidated from test_objective_finite_difference.cpp
#include "test_objective_support.hpp"
int consolidated_objective_4(){testFiniteDifference();return 0;}

// Consolidated from test_objective_fixed.cpp
#include "test_objective_support.hpp"
int consolidated_objective_5(){testFixed();return 0;}

// Consolidated from test_objective_invalid_lambda.cpp
#include "test_objective_support.hpp"
int consolidated_objective_6(){testInvalid();return 0;}

// Consolidated from test_objective_lambda_scaling.cpp
#include "test_objective_support.hpp"
int consolidated_objective_7(){testLambda();return 0;}

// Consolidated from test_objective_opposing_gradients.cpp
#include "test_objective_support.hpp"
int consolidated_objective_8(){testOpposing();return 0;}

// Consolidated from test_objective_properties.cpp
#include "test_objective_support.hpp"
int consolidated_objective_9(){testProperties();return 0;}

// Consolidated from test_objective_seed_replay.cpp
#include "test_objective_support.hpp"
int consolidated_objective_10(){testReplay();return 0;}

// Consolidated from test_objective_state_semantics.cpp
#include "test_objective_support.hpp"
int consolidated_objective_11(){testState();return 0;}

// Consolidated from test_objective_tie_and_boundary.cpp
#include "test_objective_support.hpp"
int consolidated_objective_12(){testTieBoundary();return 0;}

// Consolidated from test_objective_value.cpp
#include "test_objective_support.hpp"
int consolidated_objective_13(){testValue();return 0;}

// Consolidated from test_objective_wire_only.cpp
#include "test_objective_support.hpp"
int consolidated_objective_14(){testWireOnly();return 0;}

int main(){
  if (consolidated_objective_0()!=0) return 1;
  if (consolidated_objective_1()!=0) return 1;
  if (consolidated_objective_2()!=0) return 1;
  if (consolidated_objective_3()!=0) return 1;
  if (consolidated_objective_4()!=0) return 1;
  if (consolidated_objective_5()!=0) return 1;
  if (consolidated_objective_6()!=0) return 1;
  if (consolidated_objective_7()!=0) return 1;
  if (consolidated_objective_8()!=0) return 1;
  if (consolidated_objective_9()!=0) return 1;
  if (consolidated_objective_10()!=0) return 1;
  if (consolidated_objective_11()!=0) return 1;
  if (consolidated_objective_12()!=0) return 1;
  if (consolidated_objective_13()!=0) return 1;
  if (consolidated_objective_14()!=0) return 1;
  return 0;
}

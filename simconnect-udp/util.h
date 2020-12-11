
#pragma once

#include <cmath>

// Macro for checking result of SimConnect operations
#define ASSERT_SC_SUCCESS(expr)                          \
   if (expr != S_OK)                                     \
   {                                                     \
      printf("Error, " #expr " did not evaluate OK.\n"); \
      abort();                                           \
   }

constexpr double radians(double degrees) {
   return degrees * 0.0174533;
}

constexpr double degrees(double rad) {
   return rad * 57.2958;
}

constexpr double sign(double val) {
   return (val > 0) ? 1.0 : -1.0;
}

float randomFloat(float a, float b) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

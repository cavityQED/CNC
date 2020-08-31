#ifndef CURVE_H
#define CURVE_H

#include <vector>
#include <cmath>
#include <iostream>

#include "Types.h"

namespace curve {

static const double pi = 3.14159;

void get_curve(params_t &p, ops_t &ops, int backlash = 0);

void get_curve(params_t &p, esp_params_t &ep);

}//curve namespace

#endif

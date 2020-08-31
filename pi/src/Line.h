#ifndef LINE_H
#define LINE_H

#include <iostream>
#include <vector>
#include <cmath>

#include "Types.h"

namespace line {
	
typedef struct {
	double x_i;
	double x_f;
	double y_i;
	double y_f;
	int x_spmm;
	int y_spmm;
	double s;
} line_params_t;

typedef struct {
	int x_steps;
	int x_time;
	int x_dir;
	
	int y_steps;
	int y_time;
	int y_dir;
} line_ops_t;

void calculate_line(line_params_t &p, line_ops_t &ops);
	
void get_line(params_t &p, ops_t &ops);
	
}//line namespace

#endif

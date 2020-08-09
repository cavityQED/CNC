#ifndef CURVE_H
#define CURVE_H

#include <vector>
#include <cmath>
#include <iostream>

namespace curve {

typedef struct {
	int					num_ops;
	
	std::vector<int>	x_steps;
	std::vector<int>	x_times;
	std::vector<int>	x_dirs;
	
	std::vector<int>	y_steps;
	std::vector<int>	y_times;
	std::vector<int>	y_dirs;
} motor_ops_t;

typedef struct {
	int	r;			//Radius of the circle in mm
	int	theta_i;	//Start angle of the circle in degrees
	int	theta_f;	//End and of the circle in degrees
	int x_spmm;		//Steps per mm of the x-axis
	int y_spmm;		//Steps per mm of the y-axis
	double s;		//Surface speed in mm/s
} circle_params_t;

static const double pi = 3.14159;

/* Calculate Circle
 * 		Calculates the steps and times for each operation of circle move
 * 		Will divide up the move into 1 degree operations
 * 
 * in:
 * 	p		Circle parameters
 * 
 * out:
 * 	ops		Struct to hold the step, time, and direction info for the
 * 			circle operations
 */ 		
void calculate_circle(circle_params_t &p, motor_ops_t &ops);
	
}//curve namespace

#endif

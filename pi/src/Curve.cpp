#include "Curve.h"

namespace curve {

void calculate_circle(circle_params_t &p, motor_ops_t &ops) {
	int num_ops = p.theta_f - p.theta_i;
	ops.num_ops = num_ops;
	
	int th = p.theta_i;
	
	ops.x_steps.resize(num_ops);
	ops.x_times.resize(num_ops);
	ops.x_dirs.resize(num_ops);
	
	ops.y_steps.resize(num_ops);
	ops.y_times.resize(num_ops);
	ops.y_dirs.resize(num_ops);
	
	double pre_x = round(100*p.r*sin(th*pi/180))/100;
	double pre_y = round(100*p.r*cos(th*pi/180))/100;

	double cur_x;
	double cur_y;
	double sq;
	
	for(int i = 0; i < num_ops; i++) {
		th++;
		cur_x = round(100*p.r*sin(th*pi/180))/100;
		cur_y = round(100*p.r*cos(th*pi/180))/100;
		
		ops.x_steps[i] = (abs(100*cur_x - 100*pre_x) * p.x_spmm)/100;
		ops.y_steps[i] = (abs(100*cur_y - 100*pre_y) * p.y_spmm)/100;

		sq = sqrt(ops.x_steps[i]*ops.x_steps[i]*p.x_spmm*p.x_spmm + ops.y_steps[i]*ops.y_steps[i]*p.y_spmm*p.y_spmm);
		
		ops.x_times[i] = 1000000 * (sq / (p.s*p.x_spmm*p.y_spmm*ops.x_steps[i]));
		ops.y_times[i] = ops.x_times[i]*ops.x_steps[i]/ops.y_steps[i];
		
		ops.x_dirs[i] = !std::signbit(cur_x - pre_x);
		ops.y_dirs[i] = !std::signbit(cur_y - pre_y);
		
		pre_x = cur_x;
		pre_y = cur_y;
	}		
}//calculate_circle

}//curve namespace

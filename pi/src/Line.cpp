#include "Line.h"

namespace line {
	
void calculate_line(line_params_t &p, line_ops_t &ops) {
	double x_diff = round(100*p.x_f - 100*p.x_i)/100;
	double y_diff = round(100*p.y_f - 100*p.y_i)/100;

	ops.x_dir = !std::signbit(x_diff);
	ops.y_dir = !std::signbit(y_diff);
	
	ops.x_steps = (abs(x_diff) * p.x_spmm);	
	ops.y_steps = (abs(y_diff) * p.y_spmm);	
	
	std::cout << "X Steps: " << ops.x_steps << "\tY Steps: " << ops.y_steps << '\n';
	std::cout << "X SPMM: " << p.x_spmm << "\tY SPMM: " << p.y_spmm << '\n';
	
	double d = sqrt(x_diff*x_diff + y_diff*y_diff);
	double t = d / p.s;
	
	std::cout << "Total Time: " << t << '\n';
	
	if(ops.x_steps > 0)
		ops.x_time = t * 1000000/ops.x_steps;
	else 
		ops.x_time = 0;
		
	if(ops.y_steps > 0)
		ops.y_time = t * (1000000/ops.y_steps);
	else
		ops.y_time = 0;
	
}//calculate_line	

void get_line(params_t &p, ops_t &ops) {
	double x_diff = round(100*p.x_f - 100*p.x_i)/100;
	double y_diff = round(100*p.y_f - 100*p.y_i)/100;
	
	ops.x_dir = !std::signbit(x_diff);
	ops.y_dir = !std::signbit(y_diff);
		
	ops.x_steps = (fabs(x_diff) * (double)p.x_spmm);	
	ops.y_steps = (fabs(y_diff) * (double)p.y_spmm);	
	
	std::cout << "X Steps: " << ops.x_steps << "\tY Steps: " << ops.y_steps << '\n';
	std::cout << "X SPMM: " << p.x_spmm << "\tY SPMM: " << p.y_spmm << '\n';
	
	double d = sqrt(x_diff*x_diff + y_diff*y_diff);
	double t = d / p.speed;
	
	std::cout << "Total Time: " << t << '\n';
	
	if(ops.x_steps > 0)
		ops.x_time = t * 1000000/ops.x_steps;
	else 
		ops.x_time = 0;
		
	if(ops.y_steps > 0)
		ops.y_time = t * (1000000/ops.y_steps);
	else
		ops.y_time = 0;
}//get_line
		
}//line namespace

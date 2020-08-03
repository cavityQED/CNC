#include <vector>
#include <iostream>
#include <cmath>

int main(int argc, char* argv[]) {
	std::vector<int> x_pos;
	std::vector<double> y_pos;
	
	std::vector<int> y_steps;
	std::vector<int> y_times;
	
	std::vector<int> x_times;
	
	double SPMM = 200;
	int v = 4;
	int r = 50;
	int s_x = 200;
	
	int num_steps = r*SPMM/s_x;
	
	x_pos.resize(num_steps+1);
	y_pos.resize(num_steps+1);
	y_steps.resize(num_steps+1);
	y_times.resize(num_steps+1);
	x_times.resize(num_steps+1);
	
	x_pos[0] = r;
	y_pos[0] = 0;
	
	int y_steps_total = 0;
	for(int i = 1; i <= num_steps; i++) {
		x_pos[i] = (num_steps-i)*s_x/SPMM;
		std::cout << "x pos " << i << ": " << x_pos[i] << '\n';
		y_pos[i] = (double)round(100*sqrt((r*r) - (x_pos[i]*x_pos[i])))/100;
		std::cout << "y pos " << i << ": " << y_pos[i] << '\n';
		
		y_steps[i] = round(100*(abs(SPMM*y_pos[i] - SPMM*y_pos[i-1])))/100;
		std::cout << "y steps " << i << ": " << y_steps[i] << '\n';
		std::cout << "line\n";
		x_times[i] = (int) 1000000*sqrt((s_x*s_x) + (y_steps[i]*y_steps[i]))/(v*SPMM*s_x);
		y_times[i] = x_times[i]*s_x/y_steps[i]; 
		
		y_steps_total += y_steps[i];
	}
	
	for(int i = 0; i <= num_steps; i++) {
		std::cout << "X Time:\t" << x_times[i] << "\tY Time:\t" << y_times[i] << '\n';
	}
	
	std::cout << "Total Y Steps: " << y_steps_total << "\n";
	
	return 0;
	
}

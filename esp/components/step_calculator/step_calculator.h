#ifndef STEP_CALCULATOR
#define STEP_CALCULATOR

#include <vector>
#include <iostream>

namespace calculator {
	static double accel = 80000;					//Acceleration in steps/s/s
	static double initial_wait_time_factor = 10; 	//initial_wait_time = factor*final_wait_time
	
	void get_linear_steps_with_accel(	std::vector<bool> &step,
										int total_steps,
										int final_wait_time_us);
		
	inline void set_accel(double a) {accel = a;}
	
}//calculator namespace

#endif

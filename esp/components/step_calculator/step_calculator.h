#ifndef STEP_CALCULATOR
#define STEP_CALCULATOR

namespace calculator {
	void get_steps_with_accel(	std::vector<bool> step,
								std::vector<bool> dirs,
								int total_steps,
								int final_wait_time_us);
	
	void set_accel(int a) {accel = a;}
	
	
	int accel = 10000;
}//calculator namespace

#endif
